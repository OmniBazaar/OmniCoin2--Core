/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
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

#include <fc/smart_ref_impl.hpp>
#include <fc/network/ip.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/buyback.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/internal_exceptions.hpp>
#include <graphene/chain/special_authority.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <../omnibazaar/reserved_names_object.hpp>

#include <algorithm>

namespace graphene { namespace chain {

void verify_authority_accounts( const database& db, const authority& a )
{
   const auto& chain_params = db.get_global_properties().parameters;
   GRAPHENE_ASSERT(
      a.num_auths() <= chain_params.maximum_authority_membership,
      internal_verify_auth_max_auth_exceeded,
      "Maximum authority membership exceeded" );
   for( const auto& acnt : a.account_auths )
   {
      GRAPHENE_ASSERT( db.find_object( acnt.first ) != nullptr,
         internal_verify_auth_account_not_found,
         "Account ${a} specified in authority does not exist",
         ("a", acnt.first) );
   }
}

void verify_account_votes( const database& db, const account_options& options )
{
   // ensure account's votes satisfy requirements
   // NB only the part of vote checking that requires chain state is here,
   // the rest occurs in account_options::validate()

   const auto& gpo = db.get_global_properties();

   uint32_t max_vote_id = gpo.next_available_vote_id;
   bool has_worker_votes = false;
   for( auto id : options.votes )
   {
      FC_ASSERT( id < max_vote_id );
      has_worker_votes |= (id.type() == vote_id_type::worker);
   }

   if( has_worker_votes && (db.head_block_time() >= HARDFORK_607_TIME) )
   {
      const auto& against_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_against>();
      for( auto id : options.votes )
      {
         if( id.type() == vote_id_type::worker )
         {
            FC_ASSERT( against_worker_idx.find( id ) == against_worker_idx.end() );
         }
      }
   }
   if ( db.head_block_time() >= HARDFORK_CORE_143_TIME ) {
      const auto& approve_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_for>();
      const auto& committee_idx = db.get_index_type<committee_member_index>().indices().get<by_vote_id>();
      const auto& witness_idx = db.get_index_type<witness_index>().indices().get<by_vote_id>();
      for ( auto id : options.votes ) {
         switch ( id.type() ) {
            case vote_id_type::committee:
               FC_ASSERT( committee_idx.find(id) != committee_idx.end() );
               break;
            case vote_id_type::witness:
               FC_ASSERT( witness_idx.find(id) != witness_idx.end());
               break;
            case vote_id_type::worker:
               FC_ASSERT( approve_worker_idx.find( id ) != approve_worker_idx.end() );
               break;
            default:
               FC_THROW( "Invalid Vote Type: ${id}", ("id", id) );
               break;
         }
      }
   }
}


void_result account_create_evaluator::do_evaluate( const account_create_operation& op )
{ try {
   database& d = db();
   if( d.head_block_time() < HARDFORK_516_TIME )
   {
      FC_ASSERT( !op.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.active_special_authority.valid() );
   }
   if( d.head_block_time() < HARDFORK_599_TIME )
   {
      FC_ASSERT( !op.extensions.value.null_ext.valid() );
      FC_ASSERT( !op.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.active_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.buyback_options.valid() );
   }

   FC_ASSERT( d.find_object(op.options.voting_account), "Invalid proxy account specified." );

   try
   {
      verify_authority_accounts( d, op.owner );
      verify_authority_accounts( d, op.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_create_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_create_auth_account_not_found )

   if( op.extensions.value.owner_special_authority.valid() )
      evaluate_special_authority( d, *op.extensions.value.owner_special_authority );
   if( op.extensions.value.active_special_authority.valid() )
      evaluate_special_authority( d, *op.extensions.value.active_special_authority );
   if( op.extensions.value.buyback_options.valid() )
      evaluate_buyback_account_options( d, *op.extensions.value.buyback_options );
   verify_account_votes( d, op.options );

   auto& acnt_indx = d.get_index_type<account_index>();
   if( op.name.size() )
   {
      auto current_account_itr = acnt_indx.indices().get<by_name>().find( op.name );
      FC_ASSERT( current_account_itr == acnt_indx.indices().get<by_name>().end(), "Name is already registered." );

      FC_ASSERT( d.get_reserved_names().names.count(fc::to_lower(op.name)) <= 0, "Name is already registered." );
   }

   FC_ASSERT( op.referrer(d).is_referrer, "Can't use ${ref} as referrer, that account opted out of Referral program.",
              ("ref", op.referrer(d).name));

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type account_create_evaluator::do_apply( const account_create_operation& o )
{ try {

   database& d = db();
   uint16_t referrer_percent = o.referrer_percent;
   bool has_small_percent = (
         (db().head_block_time() <= HARDFORK_453_TIME)
      && (o.referrer != o.registrar  )
      && (o.referrer_percent != 0    )
      && (o.referrer_percent <= 0x100)
      );

   if( has_small_percent )
   {
      if( referrer_percent >= 100 )
      {
         wlog( "between 100% and 0x100%:  ${o}", ("o", o) );
      }
      referrer_percent = referrer_percent*100;
      if( referrer_percent > GRAPHENE_100_PERCENT )
         referrer_percent = GRAPHENE_100_PERCENT;
   }

   const auto& new_acnt_object = db().create<account_object>( [&]( account_object& obj ){
         obj.registrar = o.registrar;
         obj.referrer = o.referrer;

         auto& params = db().get_global_properties().parameters;
         obj.network_fee_percentage = params.network_percent_of_fee;
         obj.referrer_rewards_percentage = referrer_percent;

         obj.name             = o.name;
         obj.owner            = o.owner;
         obj.active           = o.active;
         obj.options          = o.options;
         obj.statistics = db().create<account_statistics_object>([&](account_statistics_object& s){s.owner = obj.id;}).id;

         if( o.extensions.value.owner_special_authority.valid() )
            obj.owner_special_authority = *(o.extensions.value.owner_special_authority);
         if( o.extensions.value.active_special_authority.valid() )
            obj.active_special_authority = *(o.extensions.value.active_special_authority);
         if( o.extensions.value.buyback_options.valid() )
         {
            obj.allowed_assets = o.extensions.value.buyback_options->markets;
            obj.allowed_assets->emplace( o.extensions.value.buyback_options->asset_to_buy );
         }

         obj.drive_id = o.drive_id;
         obj.mac_address = o.mac_address;
         obj.btc_address = o.btc_address;
         obj.eth_address = o.eth_address;

         if(db().head_block_time() > HARDFORK_OM_702_TIME)
         {
             obj.implicit_escrow_options.active_witness = true;
             obj.implicit_escrow_options.positive_rating = true;
             obj.implicit_escrow_options.voted_witness = true;
         }
   });

   /*
   if( has_small_percent )
   {
      wlog( "Account affected by #453 registered in block ${n}:  ${na} reg=${reg} ref=${ref}:${refp} ltr=${ltr}:${ltrp}",
         ("n", db().head_block_num()) ("na", new_acnt_object.id)
         ("reg", o.registrar) ("ref", o.referrer) ("ltr", new_acnt_object.lifetime_referrer)
         ("refp", new_acnt_object.referrer_rewards_percentage) ("ltrp", new_acnt_object.lifetime_referrer_fee_percentage) );
      wlog( "Affected account object is ${o}", ("o", new_acnt_object) );
   }
   */

   const auto& dynamic_properties = db().get_dynamic_global_properties();
   db().modify(dynamic_properties, [](dynamic_global_property_object& p) {
      ++p.accounts_registered_this_interval;
   });

   const auto& global_properties = db().get_global_properties();
   if( dynamic_properties.accounts_registered_this_interval %
       global_properties.parameters.accounts_per_fee_scale == 0 )
      db().modify(global_properties, [&dynamic_properties](global_property_object& p) {
         p.parameters.current_fees->get<account_create_operation>().basic_fee <<= p.parameters.account_fee_scale_bitshifts;
      });

   if(    o.extensions.value.owner_special_authority.valid()
       || o.extensions.value.active_special_authority.valid() )
   {
      db().create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = new_acnt_object.id;
      } );
   }

   if( o.extensions.value.buyback_options.valid() )
   {
      asset_id_type asset_to_buy = o.extensions.value.buyback_options->asset_to_buy;

      d.create< buyback_object >( [&]( buyback_object& bo )
      {
         bo.asset_to_buy = asset_to_buy;
      } );

      d.modify( asset_to_buy(d), [&]( asset_object& a )
      {
         a.buyback_account = new_acnt_object.id;
      } );
   }

   return new_acnt_object.id;
} FC_CAPTURE_AND_RETHROW((o)) }


void_result account_update_evaluator::do_evaluate( const account_update_operation& o )
{ try {
   database& d = db();
   if( d.head_block_time() < HARDFORK_516_TIME )
   {
      FC_ASSERT( !o.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !o.extensions.value.active_special_authority.valid() );
   }
   if( d.head_block_time() < HARDFORK_599_TIME )
   {
      FC_ASSERT( !o.extensions.value.null_ext.valid() );
      FC_ASSERT( !o.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !o.extensions.value.active_special_authority.valid() );
   }

   try
   {
      if( o.owner )  verify_authority_accounts( d, *o.owner );
      if( o.active ) verify_authority_accounts( d, *o.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_update_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_update_auth_account_not_found )

   if( o.extensions.value.owner_special_authority.valid() )
      evaluate_special_authority( d, *o.extensions.value.owner_special_authority );
   if( o.extensions.value.active_special_authority.valid() )
      evaluate_special_authority( d, *o.extensions.value.active_special_authority );

   acnt = &o.account(d);

   if( o.new_options.valid() )
      verify_account_votes( d, *o.new_options );

   // If user specified that he wants to be a publisher then he must also
   // either already have an address or specify it in this operation.
   if(o.is_a_publisher && *o.is_a_publisher)
   {
       const bool will_have_address = o.publisher_ip
               ? !o.publisher_ip->empty()
               : !acnt->publisher_ip.empty();
       FC_ASSERT( will_have_address, "Cannot register publisher with empty address." );
   }

   // If user specified an address then he must also become a publisher.
   if(o.publisher_ip)
   {
       const bool will_be_publisher = o.is_a_publisher
               ? *o.is_a_publisher
               : acnt->is_a_publisher;

       if(o.publisher_ip->empty())
       {
           FC_ASSERT( !will_be_publisher, "Publisher can't have empty address." );
       }
       else
       {
           FC_ASSERT( will_be_publisher, "Cannot set publisher IP while not being publisher." );

           // Check for duplicating addresses.
           const auto& idx = d.get_index_type<account_index>().indices().get<by_publisher_ip>();
           const auto itr = idx.find(*o.publisher_ip);
           FC_ASSERT( itr == idx.end(), "Address ${ip} is already registered.", ("ip", *o.publisher_ip) );
       }
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   database& d = db();
   bool sa_before, sa_after;
   d.modify( *acnt, [&](account_object& a){
      if( o.owner )
      {
         a.owner = *o.owner;
         a.top_n_control_flags = 0;
      }
      if( o.active )
      {
         a.active = *o.active;
         a.top_n_control_flags = 0;
      }
      if( o.new_options ) a.options = *o.new_options;
      sa_before = a.has_special_authority();
      if( o.extensions.value.owner_special_authority.valid() )
      {
         a.owner_special_authority = *(o.extensions.value.owner_special_authority);
         a.top_n_control_flags = 0;
      }
      if( o.extensions.value.active_special_authority.valid() )
      {
         a.active_special_authority = *(o.extensions.value.active_special_authority);
         a.top_n_control_flags = 0;
      }

      // This must be evaluated before 'publisher_ip' update.
      if (o.is_a_publisher)
      {
         a.is_a_publisher = *o.is_a_publisher;
      }

      if(o.is_an_escrow)
      {
         a.is_an_escrow = *o.is_an_escrow;
      }

      if(o.escrow_fee)
      {
          a.escrow_fee = *o.escrow_fee;
      }

      if(o.escrows)
      {
          a.escrows = *o.escrows;
      }

      // This must be evaluated after 'is_a_publisher' update.
      if(a.is_a_publisher)
      {
          if(o.publisher_ip)
          {
              a.publisher_ip = *o.publisher_ip;
          }
      }
      else
      {
          // Clear IP address when account is not a publisher anymore.
          a.publisher_ip = string();
      }

      if(o.implicit_escrow_options)
      {
          a.implicit_escrow_options = *o.implicit_escrow_options;
      }

      if(o.is_referrer)
      {
          a.is_referrer = *o.is_referrer;
      }

      if(o.btc_address)
      {
          a.btc_address = *o.btc_address;
      }

      if(o.eth_address)
      {
          a.eth_address = *o.eth_address;
      }

      if(o.publisher_fee)
      {
          a.publisher_fee = *o.publisher_fee;
      }
            
      sa_after = a.has_special_authority();
   });

   if( sa_before && (!sa_after) )
   {
      const auto& sa_idx = d.get_index_type< special_authority_index >().indices().get<by_account>();
      auto sa_it = sa_idx.find( o.account );
      assert( sa_it != sa_idx.end() );
      d.remove( *sa_it );
   }
   else if( (!sa_before) && sa_after )
   {
      d.create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = o.account;
      } );
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_evaluate(const account_whitelist_operation& o)
{ try {
   database& d = db();

   listed_account = &o.account_to_list(d);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_apply(const account_whitelist_operation& o)
{ try {
   database& d = db();

   d.modify(*listed_account, [&o](account_object& a) {
      if( o.new_listing & o.white_listed )
         a.whitelisting_accounts.insert(o.authorizing_account);
      else
         a.whitelisting_accounts.erase(o.authorizing_account);

      if( o.new_listing & o.black_listed )
         a.blacklisting_accounts.insert(o.authorizing_account);
      else
         a.blacklisting_accounts.erase(o.authorizing_account);
   });

   /** for tracking purposes only, this state is not needed to evaluate */
   d.modify( o.authorizing_account(d), [&]( account_object& a ) {
     if( o.new_listing & o.white_listed )
        a.whitelisted_accounts.insert( o.account_to_list );
     else
        a.whitelisted_accounts.erase( o.account_to_list );

     if( o.new_listing & o.black_listed )
        a.blacklisted_accounts.insert( o.account_to_list );
     else
        a.blacklisted_accounts.erase( o.account_to_list );
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
