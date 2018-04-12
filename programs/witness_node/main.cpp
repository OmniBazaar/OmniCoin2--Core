/*
 * Copyright (c) 2015-2017 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/app/application.hpp>

#include <graphene/witness/witness.hpp>
#include <graphene/debug_witness/debug_witness.hpp>
#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/elasticsearch/elasticsearch_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>
#include <graphene/delayed_node/delayed_node_plugin.hpp>
#include <graphene/snapshot/snapshot.hpp>

#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>
#include <fc/interprocess/signals.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/logger.hpp>
#include <fc/log/logger_config.hpp>

#include <boost/filesystem.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/container/flat_set.hpp>

#include <iostream>
#include <fstream>

#ifdef WIN32
# include <signal.h> 
#else
# include <csignal>
#endif

using namespace graphene;
namespace bpo = boost::program_options;

void write_default_logging_config(const fc::path& filepath);
fc::optional<fc::logging_config> load_logging_config_from_ini_file(const fc::path& config_ini_filename);

class deduplicator 
{
   public:
      deduplicator() : modifier(nullptr) {}

      deduplicator(const boost::shared_ptr<bpo::option_description> (*mod_fn)(const boost::shared_ptr<bpo::option_description>&))
              : modifier(mod_fn) {}

      const boost::shared_ptr<bpo::option_description> next(const boost::shared_ptr<bpo::option_description>& o)
      {
         const std::string name = o->long_name();
         if( seen.find( name ) != seen.end() )
            return nullptr;
         seen.insert(name);
         return modifier ? modifier(o) : o;
      }

   private:
      boost::container::flat_set<std::string> seen;
      const boost::shared_ptr<bpo::option_description> (*modifier)(const boost::shared_ptr<bpo::option_description>&);
};

static void load_config_file( const fc::path& config_ini_path, const bpo::options_description& cfg_options,
                              bpo::variables_map& options )
{
   deduplicator dedup;
   bpo::options_description unique_options("Graphene Witness Node");
   for( const boost::shared_ptr<bpo::option_description> opt : cfg_options.options() )
   {
      const boost::shared_ptr<bpo::option_description> od = dedup.next(opt);
      if( !od ) continue;
      unique_options.add( od );
   }

   // get the basic options
   bpo::store(bpo::parse_config_file<char>(config_ini_path.preferred_string().c_str(),
              unique_options, true), options);
}

void load_log_config_file(const fc::path& config_path)
{
    // try to get logging options from the config file.
    try
    {
       fc::optional<fc::logging_config> logging_config = load_logging_config_from_ini_file(config_path);
       if (logging_config)
          fc::configure_logging(*logging_config);
    }
    catch (const fc::exception&)
    {
       wlog("Error parsing logging config from config file ${config}, using default config", ("config", config_path.preferred_string()));
    }
}

const boost::shared_ptr<bpo::option_description> new_option_description( const std::string& name, const bpo::value_semantic* value, const std::string& description )
{
    bpo::options_description helper("");
    helper.add_options()( name.c_str(), value, description.c_str() );
    return helper.options()[0];
}

static void create_new_config_file( const fc::path& config_ini_path, const fc::path& data_dir,
                                    const bpo::options_description& cfg_options )
{
   ilog("Writing new config file at ${path}", ("path", config_ini_path));
   if( !fc::exists(data_dir) )
      fc::create_directories(data_dir);

   auto modify_option_defaults = [](const boost::shared_ptr<bpo::option_description>& o) -> const boost::shared_ptr<bpo::option_description> {
       const std::string& name = o->long_name();
       if( name == "partial-operations" )
          return new_option_description( name, bpo::value<bool>()->default_value(true), o->description() );
       if( name == "max-ops-per-account" )
          return new_option_description( name, bpo::value<int>()->default_value(1000), o->description() );
       return o;
   };
   deduplicator dedup(modify_option_defaults);
   std::ofstream out_cfg(config_ini_path.preferred_string());
   for( const boost::shared_ptr<bpo::option_description> opt : cfg_options.options() )
   {
      const boost::shared_ptr<bpo::option_description> od = dedup.next(opt);
      if( !od ) continue;

      if( !od->description().empty() )
         out_cfg << "# " << od->description() << "\n";
      boost::any store;
      if( !od->semantic()->apply_default(store) )
         out_cfg << "# " << od->long_name() << " = \n";
      else {
         auto example = od->format_parameter();
         if( example.empty() )
            // This is a boolean switch
            out_cfg << od->long_name() << " = " << "false\n";
         else {
            // The string is formatted "arg (=<interesting part>)"
            example.erase(0, 6);
            example.erase(example.length()-1);
            out_cfg << od->long_name() << " = " << example << "\n";
         }
      }
      out_cfg << "\n";
   }
   out_cfg.close();
}

void create_new_log_config_file(const fc::path& config_path)
{
    write_default_logging_config(config_path);

    // read the default logging config we just wrote out to the file and start using it
    fc::optional<fc::logging_config> logging_config = load_logging_config_from_ini_file(config_path);
    if (logging_config)
       fc::configure_logging(*logging_config);
}

int main(int argc, char** argv) {
   app::application* node = new app::application();
   fc::oexception unhandled_exception;
   try {
      bpo::options_description app_options("Graphene Witness Node");
      bpo::options_description cfg_options("Graphene Witness Node");
      app_options.add_options()
            ("help,h", "Print this help message and exit.")
            ("data-dir,d", bpo::value<boost::filesystem::path>()->default_value("witness_node_data_dir"), "Directory containing databases, configuration file, etc.")
            ;

      bpo::variables_map options;

      auto witness_plug = node->register_plugin<witness_plugin::witness_plugin>();
      auto debug_witness_plug = node->register_plugin<debug_witness_plugin::debug_witness_plugin>();
      auto history_plug = node->register_plugin<account_history::account_history_plugin>();
      auto elasticsearch_plug = node->register_plugin<elasticsearch::elasticsearch_plugin>();
      auto market_history_plug = node->register_plugin<market_history::market_history_plugin>();
      auto delayed_plug = node->register_plugin<delayed_node::delayed_node_plugin>();
      auto snapshot_plug = node->register_plugin<snapshot_plugin::snapshot_plugin>();

      try
      {
         bpo::options_description cli, cfg;
         node->set_program_options(cli, cfg);
         app_options.add(cli);
         cfg_options.add(cfg);
         bpo::store(bpo::parse_command_line(argc, argv, app_options), options);
      }
      catch (const boost::program_options::error& e)
      {
        std::cerr << "Error parsing command line: " << e.what() << "\n";
        return 1;
      }

      if( options.count("help") )
      {
         std::cout << app_options << "\n";
         return 0;
      }

      fc::path data_dir;
      if( options.count("data-dir") )
      {
         data_dir = options["data-dir"].as<boost::filesystem::path>();
         if( data_dir.is_relative() )
            data_dir = fc::current_path() / data_dir;
      }

      const fc::path config_ini_path = data_dir / "config.ini";
      if( !fc::exists(config_ini_path) )
         create_new_config_file( config_ini_path, data_dir, cfg_options );
      load_config_file( config_ini_path, cfg_options, options );

      const fc::path log_config_ini_path = data_dir / "config_log.ini";
      if( !fc::exists(log_config_ini_path) )
         create_new_log_config_file(log_config_ini_path);
      load_log_config_file(log_config_ini_path);

      bpo::notify(options);
      node->initialize(data_dir, options);
      node->initialize_plugins( options );

      node->startup();
      node->startup_plugins();

      fc::promise<int>::ptr exit_promise = new fc::promise<int>("UNIX Signal Handler");

      fc::set_signal_handler([&exit_promise](int signal) {
         elog( "Caught SIGINT attempting to exit cleanly" );
         exit_promise->set_value(signal);
      }, SIGINT);

      fc::set_signal_handler([&exit_promise](int signal) {
         elog( "Caught SIGTERM attempting to exit cleanly" );
         exit_promise->set_value(signal);
      }, SIGTERM);

      ilog("Started BitShares node on a chain with ${h} blocks.", ("h", node->chain_database()->head_block_num()));
      ilog("Chain ID is ${id}", ("id", node->chain_database()->get_chain_id()) );

      int signal = exit_promise->wait();
      ilog("Exiting from signal ${n}", ("n", signal));
      node->shutdown_plugins();
      node->shutdown();
      delete node;
      return 0;
   } catch( const fc::exception& e ) {
      // deleting the node can yield, so do this outside the exception handler
      unhandled_exception = e;
   }

   if (unhandled_exception)
   {
      elog("Exiting with error:\n${e}", ("e", unhandled_exception->to_detail_string()));
      node->shutdown();
      delete node;
      return 1;
   }
}

void write_default_logging_config(const fc::path& filepath)
{
    const fc::mutable_variant_object console_appenders = fc::mutable_variant_object
            ("stderr", fc::console_appender::config())
            ;
    const fc::mutable_variant_object file_appenders = fc::mutable_variant_object
            ("p2p", [](){
                fc::file_appender::config cfg("logs/p2p/p2p.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("all", [](){
                fc::file_appender::config cfg("logs/all.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("bonus", [](){
                fc::file_appender::config cfg("logs/bonus.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("escrow", [](){
                fc::file_appender::config cfg("logs/escrow.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("mail", [](){
                fc::file_appender::config cfg("logs/mail.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("pop", [](){
                fc::file_appender::config cfg("logs/pop.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ("market", [](){
                fc::file_appender::config cfg("logs/market.log");
                cfg.rotate = true;
                cfg.rotation_interval = fc::hours(1);
                cfg.rotation_limit = fc::days(1);
                return cfg;
            }())
            ;
    const fc::mutable_variant_object loggers = fc::mutable_variant_object
            ("default", [](){
                fc::logger_config cfg("default");
                cfg.appenders.push_back("stderr");
                cfg.appenders.push_back("all");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("p2p", [](){
                fc::logger_config cfg("p2p");
                cfg.appenders.push_back("p2p");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("bonus", [](){
                fc::logger_config cfg("bonus");
                cfg.appenders.push_back("bonus");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("escrow", [](){
                fc::logger_config cfg("escrow");
                cfg.appenders.push_back("escrow");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("mail", [](){
                fc::logger_config cfg("mail");
                cfg.appenders.push_back("mail");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("pop", [](){
                fc::logger_config cfg("pop");
                cfg.appenders.push_back("pop");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ("market", [](){
                fc::logger_config cfg("market");
                cfg.appenders.push_back("market");
                cfg.level = fc::log_level::info;
                return cfg;
            }())
            ;

    const fc::mutable_variant_object config = fc::mutable_variant_object
            (fc::get_typename<fc::console_appender::config>::name(), console_appenders)
            (fc::get_typename<fc::file_appender::config>::name(), file_appenders)
            (fc::get_typename<fc::logger_config>::name(), loggers)
            ;

    fc::json::save_to_file(config, filepath);
}

fc::optional<fc::logging_config> load_logging_config_from_ini_file(const fc::path& config_ini_filename)
{
   try
   {
      fc::logging_config logging_config;

      const fc::mutable_variant_object config = fc::json::from_file(config_ini_filename).as<fc::mutable_variant_object>();

      for(const auto& appender : config[fc::get_typename<fc::console_appender::config>::name()].as<fc::mutable_variant_object>())
      {
          logging_config.appenders.push_back(fc::appender_config(appender.key(), "console", appender.value()));
      }

      for(const auto& appender : config[fc::get_typename<fc::file_appender::config>::name()].as<fc::mutable_variant_object>())
      {
          logging_config.appenders.push_back(fc::appender_config(appender.key(), "file", appender.value()));
      }

      for(const auto& logger : config[fc::get_typename<fc::logger_config>::name()].as<fc::mutable_variant_object>())
      {
          logging_config.loggers.push_back(logger.value().as<fc::logger_config>());
      }

      return logging_config;
   }
   FC_RETHROW_EXCEPTIONS(warn, "")
}
