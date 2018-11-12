#include <mail_controller.hpp>
#include <mail_storage.hpp>
#include <mail_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/app/application.hpp>
#include <graphene/app/api.hpp>
#include <fc/thread/scoped_lock.hpp>

static const int TIMER_TICK_INTERVAL_IN_SECONDS = 1;

namespace omnibazaar {

    mail_controller::mail_controller(graphene::app::application& app)
        : _app(app)
        , _thread("mail")
    {
        mail_ilog("Connecting mail signals.");
        _new_mail_connection = _app.p2p_node()->mail_new.connect([this](const mail_object& m){ on_new_mail(m); });
        _received_mail_connection = _app.p2p_node()->mail_received.connect([this](const std::string& u){ on_mail_received(u); });
        _confirm_received_mail_connection = _app.p2p_node()->mail_confirm_received.connect([this](const std::string& u){ on_mail_confirm_received(u); });

        mail_ilog("Starting mail processing loop.");
        start_mail_processing_loop();
    }

    mail_controller::~mail_controller()
    {
        mail_ilog("Terminating mail background thread.");
        _thread.quit();
    }

    void mail_controller::send(callback_type cb, const mail_object& mail)
    {
        mail_ddump((cb)(mail));

        {
            // Register callback for notifying sender about delivery.
            const fc::scoped_lock<fc::spin_lock> lock(_send_callbacks_lock);
            _send_callbacks[mail.uuid] = cb;
        }
        // Save mail to disk.
        _app.mail_storage()->store(mail);
        // Send mail to other backend nodes.
        _app.p2p_node()->mail_send(mail);
        // If receiving user is connected to this node, send mail directly.
        const auto itr = _receive_callbacks.find(mail.recipient);
        if(itr != _receive_callbacks.end())
        {
            exec_callback(itr->second, { fc::variant(mail) });
        }
    }

    void mail_controller::subscribe(callback_type cb, const std::string& receiver_name)
    {
        mail_ddump((cb)(receiver_name));

        {
            // Register callback.
            const fc::scoped_lock<fc::spin_lock> lock(_receive_callbacks_lock);
            _receive_callbacks[receiver_name] = cb;
        }
        // Send any pending mails to this callback.
        const std::vector<mail_object> mails = _app.mail_storage()->get_mails_by_receiver(receiver_name);
        std::vector<fc::variant> data;
        data.reserve(mails.size());
        std::transform(mails.begin(), mails.end(), std::back_inserter(data), [](const mail_object& m){ return fc::variant(m); });
        exec_callback(cb, data);
    }

    void mail_controller::set_received(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        // Move mail to delivered folder.
        _app.mail_storage()->set_received(mail_uuid);
        // Send notification that mail was received.
        _app.p2p_node()->mail_send_received(mail_uuid);
        // If sender is connected to this node, notify about successful mail delivery.
        const auto itr = _send_callbacks.find(mail_uuid);
        if(itr != _send_callbacks.end())
        {
            exec_callback(itr->second, { fc::variant(graphene::app::network_broadcast_api::send_confirmation{mail_uuid}) });
        }
    }

    void mail_controller::confirm_received(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        // Mail is now fully sent and confirmed by both sides, remove it from storage.
        _app.mail_storage()->remove(mail_uuid);
        // Send notification to other nodes.
        _app.p2p_node()->mail_send_confirm_received(mail_uuid);
    }

    void mail_controller::start_mail_processing_loop()
    {
        if(!_thread.is_running())
        {
            mail_wlog("Thread is not running.");
            return;
        }

        // Schedule operation in background thread.
        auto future = _thread.schedule([this](){ mail_sending_tick(); },
            fc::time_point::now() + fc::seconds(TIMER_TICK_INTERVAL_IN_SECONDS),
            "Executing mail processing tick");

        // Restart operation upon its completion.
        future.on_complete([this](const fc::exception_ptr& exptr) { start_mail_processing_loop(); });
    }

    void mail_controller::mail_sending_tick()
    {
        mail_dlog("Processing pending mails.");
        // Re-send mails that were not successfully delivered to receivers.
        const std::vector<fc::path> pending_mails = _app.mail_storage()->get_pending_mails();
        for(const auto& path : pending_mails)
        {
            mail_dlog("Processing mail file '${file}'.", ("file", path));
            if(!fc::exists(path))
            {
                mail_wlog("File '${path}' does not exist.", ("path", path));
                continue;
            }

            const fc::variant var = fc::json::from_file(path);
            if(var.is_null())
            {
                mail_wlog("Unable to read mail object ${obj} from '${path}'.", ("obj", var)("path", path));
                continue;
            }

            const mail_object mail = var.as<mail_object>();
            // Send mail to other backend nodes.
            _app.p2p_node()->mail_send(mail);
            // If receiving user is connected to this node, send mail directly.
            callback_type cb = nullptr;
            {
                const fc::scoped_lock<fc::spin_lock> lock(_receive_callbacks_lock);
                const auto itr = _receive_callbacks.find(mail.recipient);
                if(itr != _receive_callbacks.end())
                {
                    cb = itr->second;
                }
            }
            if(cb)
            {
                exec_callback(cb, { fc::variant(mail) });
            }
            else
            {
                mail_wlog("Unable to find receive callback for '${mail}'.", ("mail", mail.recipient));
            }
        }

        mail_dlog("Processing delivered mails.");
        // Re-send notifications to senders for mails that were delivered to receivers.
        const std::vector<std::string> pending_notifications = _app.mail_storage()->get_received_mails();
        for(const auto& mail_uuid : pending_notifications)
        {
            // Send notification that mail was received.
            _app.p2p_node()->mail_send_received(mail_uuid);
            // If sender is connected to this node, notify about successful mail delivery.
            callback_type cb = nullptr;
            {
                const fc::scoped_lock<fc::spin_lock> lock(_send_callbacks_lock);
                const auto itr = _send_callbacks.find(mail_uuid);
                if(itr != _send_callbacks.end())
                {
                    cb = itr->second;
                }
            }
            if(cb)
            {
                exec_callback(cb, { fc::variant(graphene::app::network_broadcast_api::send_confirmation{mail_uuid}) });
            }
            else
            {
                mail_wlog("Unable to find send callback for '${mail}'.", ("mail", mail_uuid));
            }
        }
    }

    void mail_controller::on_new_mail(const mail_object& mail)
    {
        mail_ddump((mail));

        if(mail.uuid.empty())
        {
            mail_wlog("Invalid mail object ${mail}.", ("mail", mail));
            return;
        }

        // Find receiver.
        const auto iter = _receive_callbacks.find(mail.recipient);
        if(iter == _receive_callbacks.end())
        {
            mail_wlog("Receiver ${rec} has no registered callback.", ("rec", mail.recipient));
            return;
        }

        exec_callback(iter->second, { fc::variant(mail) });
    }

    void mail_controller::on_mail_received(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        if(mail_uuid.empty())
        {
            mail_wlog("Mail UUID is empty.");
            return;
        }

        if(_send_callbacks.empty())
        {
            mail_wlog("Callback list is empty.");
            return;
        }

        // Find the callback.
        const auto iter = _send_callbacks.find(mail_uuid);
        if(iter == _send_callbacks.end())
        {
            mail_wlog("No callback registered for mail ${uuid}.", ("uuid", mail_uuid));
            return;
        }

        exec_callback(iter->second, { fc::variant(graphene::app::network_broadcast_api::send_confirmation{mail_uuid}) });
    }

    void mail_controller::on_mail_confirm_received(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        if(mail_uuid.empty())
        {
            mail_wlog("Mail UUID is empty.");
            return;
        }

        // Mail reception is confirmed by sender, remove it from storage.
        _app.mail_storage()->remove(mail_uuid);
    }

    void mail_controller::exec_callback(callback_type callback, const std::vector<fc::variant> &objects)
    {
        mail_ddump((callback)(objects));

        // Need to ensure the object is not deleted for the life of the async operation.
        auto capture_this = shared_from_this();

        // Invoke the callback.
        for(auto& object : objects)
        {
            fc::async( [capture_this, this, object, callback](){
               callback( object );
            } );
        }
    }

    void mail_controller::remove_send_callbacks(const std::unordered_map<std::string, callback_type>& callbacks)
    {
        mail_ddump((callbacks));

        for(auto itr = callbacks.cbegin(); itr != callbacks.cend(); ++itr)
        {
            const fc::scoped_lock<fc::spin_lock> lock(_send_callbacks_lock);
            _send_callbacks.erase(itr->first);
        }
    }

    void mail_controller::remove_receive_callbacks(const std::unordered_map<std::string, callback_type>& callbacks)
    {
        mail_ddump((callbacks));

        for(auto itr = callbacks.cbegin(); itr != callbacks.cend(); ++itr)
        {
            const fc::scoped_lock<fc::spin_lock> lock(_receive_callbacks_lock);
            _receive_callbacks.erase(itr->first);
        }
    }

}
