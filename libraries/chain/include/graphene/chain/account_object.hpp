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
#pragma once
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/db/generic_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <../omnibazaar/account_object_components.hpp>
#include <string>
#include <functional>

namespace graphene { namespace chain {
   class database;

   share_type cut_fee(share_type a, uint16_t p);

   /**
    * @class account_statistics_object
    * @ingroup object
    * @ingroup implementation
    *
    * This object contains regularly updated statistical data about an account. It is provided for the purpose of
    * separating the account data that changes frequently from the account data that is mostly static, which will
    * minimize the amount of data that must be backed up as part of the undo history everytime a transfer is made.
    */
   class account_statistics_object : public graphene::db::abstract_object<account_statistics_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_account_statistics_object_type;

         account_id_type  owner;

         /**
          * Keep the most recent operation as a root pointer to a linked list of the transaction history.
          */
         account_transaction_history_id_type most_recent_op;
         /** Total operations related to this account. */
         uint32_t                            total_ops = 0;
         /** Total operations related to this account that has been removed from the database. */
         uint32_t                            removed_ops = 0;

         /**
          * When calculating votes it is necessary to know how much is stored in orders (and thus unavailable for
          * transfers). Rather than maintaining an index of [asset,owner,order_id] we will simply maintain the running
          * total here and update it every time an order is created or modified.
          */
         share_type total_core_in_orders;

         /**
          * Tracks the total fees paid by this account for the purpose of calculating bulk discounts.
          */
         share_type lifetime_fees_paid;

         /**
          * Tracks the fees paid by this account which have not been disseminated to the various parties that receive
          * them yet (registrar, referrer, lifetime referrer, network, etc). This is used as an optimization to avoid
          * doing massive amounts of uint128 arithmetic on each and every operation.
          *
          * These fees will be paid out as vesting cash-back, and this counter will reset during the maintenance
          * interval.
          */
         share_type pending_fees;
         /**
          * Same as @ref pending_fees, except these fees will be paid out as pre-vested cash-back (immediately
          * available for withdrawal) rather than requiring the normal vesting period.
          */
         share_type pending_vested_fees;

         /// map<account, pair<vote value, asset>> used to store transaction votes and calculate Reputation Score for Proof of Participation.
         map<account_id_type, std::pair<uint16_t, asset>> reputation_votes;

         /// Set of accounts which received positive reputation vote from this account.
         set<account_id_type> my_reputation_votes;

         /// @brief Split up and pay out @ref pending_fees and @ref pending_vested_fees
         void process_fees(const account_object& a, database& d) const;

         /**
          * Core fees are paid into the account_statistics_object by this method
          */
         void pay_fee( share_type core_fee, share_type cashback_vesting_threshold );
   };

   /**
    * @brief Tracks the balance of a single account/asset pair
    * @ingroup object
    *
    * This object is indexed on owner and asset_type so that black swan
    * events in asset_type can be processed quickly.
    */
   class account_balance_object : public abstract_object<account_balance_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_account_balance_object_type;

         account_id_type   owner;
         asset_id_type     asset_type;
         share_type        balance;

         asset get_balance()const { return asset(balance, asset_type); }
         void  adjust_balance(const asset& delta);
   };

   /**
    * @brief This class represents an account on the object graph
    * @ingroup object
    * @ingroup protocol
    *
    * Accounts are the primary unit of authority on the graphene system. Users must have an account in order to use
    * assets, trade in the markets, vote for committee_members, etc.
    */
   class account_object : public graphene::db::abstract_object<account_object>
   {
      public:

         static const uint8_t space_id = protocol_ids;
         static const uint8_t type_id  = account_object_type;

         ///The account that paid the fee to register this account. Receives a percentage of referral rewards.
         account_id_type registrar;
         /// The account credited as referring this account. Receives a percentage of referral rewards.
         account_id_type referrer;

         /// Percentage of fee which should go to network.
         uint16_t network_fee_percentage = GRAPHENE_DEFAULT_NETWORK_PERCENT_OF_FEE;
         /// Percentage of referral rewards (leftover fee after paying network and lifetime referrer) which should go
         /// to referrer. The remainder of referral rewards goes to the registrar.
         uint16_t referrer_rewards_percentage = 0;

         /// The account's name. This name must be unique among all account names on the graph. May not be empty.
         string name;

         /**
          * The owner authority represents absolute control over the account. Usually the keys in this authority will
          * be kept in cold storage, as they should not be needed very often and compromise of these keys constitutes
          * complete and irrevocable loss of the account. Generally the only time the owner authority is required is to
          * update the active authority.
          */
         authority owner;
         /// The owner authority contains the hot keys of the account. This authority has control over nearly all
         /// operations the account may perform.
         authority active;

         typedef account_options  options_type;
         account_options options;

         /// The reference implementation records the account's statistics in a separate object. This field contains the
         /// ID of that object.
         account_statistics_id_type statistics;

         /**
          * This is a set of all accounts which have 'whitelisted' this account. Whitelisting is only used in core
          * validation for the purpose of authorizing accounts to hold and transact in whitelisted assets. This
          * account cannot update this set, except by transferring ownership of the account, which will clear it. Other
          * accounts may add or remove their IDs from this set.
          */
         flat_set<account_id_type> whitelisting_accounts;

         /**
          * Optionally track all of the accounts this account has whitelisted or blacklisted, these should
          * be made Immutable so that when the account object is cloned no deep copy is required.  This state is
          * tracked for GUI display purposes.
          *
          * TODO: move white list tracking to its own multi-index container rather than having 4 fields on an
          * account.   This will scale better because under the current design if you whitelist 2000 accounts,
          * then every time someone fetches this account object they will get the full list of 2000 accounts.
          */
         ///@{
         set<account_id_type> whitelisted_accounts;
         set<account_id_type> blacklisted_accounts;
         ///@}


         /**
          * This is a set of all accounts which have 'blacklisted' this account. Blacklisting is only used in core
          * validation for the purpose of forbidding accounts from holding and transacting in whitelisted assets. This
          * account cannot update this set, and it will be preserved even if the account is transferred. Other accounts
          * may add or remove their IDs from this set.
          */
         flat_set<account_id_type> blacklisting_accounts;

         /**
          * Vesting balance which receives cashback_reward deposits.
          */
         optional<vesting_balance_id_type> cashback_vb;
         // Vesting balance which receives Escrow fees.
         optional<vesting_balance_id_type> escrow_vb;
         // Vesting balance which receives Publisher fees.
         optional<vesting_balance_id_type> publisher_vb;
         // Vesting balance which receives Founder fee from sales.
         optional<vesting_balance_id_type> founder_sale_vb;
         // Vesting balance which receives Referrer fee from sales.
         optional<vesting_balance_id_type> referrer_sale_vb;

         special_authority owner_special_authority = no_special_authority();
         special_authority active_special_authority = no_special_authority();

         /**
          * This flag is set when the top_n logic sets both authorities,
          * and gets reset when authority or special_authority is set.
          */
         uint8_t top_n_control_flags = 0;
         static const uint8_t top_n_control_owner  = 1;
         static const uint8_t top_n_control_active = 2;

         /**
          * This is a set of assets which the account is allowed to have.
          * This is utilized to restrict buyback accounts to the assets that trade in their markets.
          * In the future we may expand this to allow accounts to e.g. voluntarily restrict incoming transfers.
          */
         optional< flat_set<asset_id_type> > allowed_assets;

         bool has_special_authority()const
         {
            return (owner_special_authority.which() != special_authority::tag< no_special_authority >::value)
                || (active_special_authority.which() != special_authority::tag< no_special_authority >::value);
         }

         template<typename DB>
         const vesting_balance_object& cashback_balance(const DB& db)const
         {
            FC_ASSERT(cashback_vb);
            return db.get(*cashback_vb);
         }

         account_id_type get_id()const { return id; }

         // Hardware information used to determine Welcome Bonus eligibility.
         // These members are not added to FC_REFLECT to avoid easy access by random users.
         string drive_id;
         string mac_address;

         bool received_welcome_bonus = false;

         // Flag to indicate if the account has chosen to be a publisher
         bool is_a_publisher = false;

         // Currently registered IP/domain address of publisher node
         string publisher_ip;

         // Flag to indicate if the account has chosen to be a escrow
         bool is_an_escrow = false;

         // Fee % collected by this account as an escrow agent.
         uint16_t escrow_fee = GRAPHENE_1_PERCENT / 2;

         // Users that bought something from this account. Used in Sale Bonus processing.
         std::set<account_id_type> buyers;

         // Users that this account added to acceptable Escrow agents.
         std::set<account_id_type> escrows;

         // Options for implicitly approving certain types of escrow accounts.
         escrow_options implicit_escrow_options;

         // Stores number of listings hosted by this user if this account is a publisher.
         uint64_t listings_count = 0;

         // Flag indicating that this account sent Referral bonus to its referrer.
         bool sent_referral_bonus = false;

         // Flag indicating if user opted-in to Referral program.
         bool is_referrer = true;

         // Fee paid by seller to publisher for listing hosting.
         uint16_t publisher_fee = GRAPHENE_1_PERCENT / 4;

         // Bitcoin address of this user.
         string btc_address;

         // Ethereum address of this user.
         string eth_address;

         // Proof of Participation scores in GRAPHENE_1_PERCENT.
         uint16_t referral_score = 0;
         uint16_t listings_score = 0;
         uint16_t reputation_score = 0;
         uint16_t reputation_unweighted_score = 0;
         uint16_t trust_score = 0;
         // [OM-295]: witnesses start with 100% reliability score,
         // so that they are not at a disadvantage from the start.
         uint16_t reliability_score = GRAPHENE_100_PERCENT;
         uint16_t pop_score = 0;
         // Number of reputation votes for this account.
         uint64_t reputation_votes_count = 0;

         // Flag indicating if this account passed KYC verification.
         bool verified = false;

         // Update reputation for this account given by 'from' account.
         static void update_reputation(database& db, const account_id_type target, const account_id_type from, const uint16_t reputation, const asset amount);
   };


   /**
    *  @brief This secondary index will allow a reverse lookup of all accounts that a particular key or account
    *  is an potential signing authority.
    */
   class account_member_index : public secondary_index
   {
      public:
         virtual void object_inserted( const object& obj ) override;
         virtual void object_removed( const object& obj ) override;
         virtual void about_to_modify( const object& before ) override;
         virtual void object_modified( const object& after  ) override;


         /** given an account or key, map it to the set of accounts that reference it in an active or owner authority */
         map< account_id_type, set<account_id_type> > account_to_account_memberships;
         map< public_key_type, set<account_id_type> > account_to_key_memberships;
         /** some accounts use address authorities in the genesis block */
         map< address, set<account_id_type> >         account_to_address_memberships;


      protected:
         set<account_id_type>  get_account_members( const account_object& a )const;
         set<public_key_type>  get_key_members( const account_object& a )const;
         set<address>          get_address_members( const account_object& a )const;

         set<account_id_type>  before_account_members;
         set<public_key_type>  before_key_members;
         set<address>          before_address_members;
   };


   /**
    *  @brief This secondary index will allow a reverse lookup of all accounts that have been referred by
    *  a particular account.
    */
   class account_referrer_index : public secondary_index
   {
      public:
         virtual void object_inserted( const object& obj ) override;
         virtual void object_removed( const object& obj ) override;
         virtual void about_to_modify( const object& before ) override;
         virtual void object_modified( const object& after  ) override;

         /** maps the referrer to the set of accounts that they have referred */
         map< account_id_type, set<account_id_type> > referred_by;
   };

   /* structure that contains just account name and id */
   struct account_object_name {

	   account_id_type id;
	   std::string name;
	   
	   account_object_name() {}

	   account_object_name(account_id_type id, const std::string& name)
	   {
		   this->id = id;
		   this->name = name;
	   }
   };

   /* class comparer for account_object_name */
   class account_object_name_comparer
   {
   public:
	   bool operator()(const account_object_name& a, const std::string& bName) const
	   {
		   return a.name.compare(bName) < 0;
	   }

	   bool operator()(const std::string& aName, const account_object& b) const
	   {
		   return aName.compare(b.name) < 0;
	   }
   };


   /**
    *  @brief This secondary index will allow lookup of Escrow agents.
    */
   class account_escrow_index : public secondary_index
   {
      public:
         virtual void object_inserted( const object& obj ) override;
         virtual void object_removed( const object& obj ) override;
         virtual void about_to_modify( const object& before ) override;
         virtual void object_modified( const object& after  ) override;

		 // get escrow names that start with search_term, paginated by start and limit
         std::vector<account_object_name> filter_by_name(uint32_t start, uint32_t limit, const std::string& search_term) const;

		 // list of objects that contain username and id of the account
         std::vector<account_object_name> current_escrows;

		private:
			void insert_keeping_sorted(const account_object_name& account_object);
   };

   struct by_account_asset;
   struct by_asset_balance;
   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      account_balance_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >,
         ordered_unique< tag<by_account_asset>,
            composite_key<
               account_balance_object,
               member<account_balance_object, account_id_type, &account_balance_object::owner>,
               member<account_balance_object, asset_id_type, &account_balance_object::asset_type>
            >
         >,
         ordered_unique< tag<by_asset_balance>,
            composite_key<
               account_balance_object,
               member<account_balance_object, asset_id_type, &account_balance_object::asset_type>,
               member<account_balance_object, share_type, &account_balance_object::balance>,
               member<account_balance_object, account_id_type, &account_balance_object::owner>
            >,
            composite_key_compare<
               std::less< asset_id_type >,
               std::greater< share_type >,
               std::less< account_id_type >
            >
         >
      >
   > account_balance_object_multi_index_type;

   /**
    * @ingroup object_index
    */
   typedef generic_index<account_balance_object, account_balance_object_multi_index_type> account_balance_index;

   struct by_name{};
   struct by_reputation_votes;
   struct by_publishers;
   struct by_listings_count;
   struct by_publisher_ip;
   struct by_hardware_info;

   /**
    * @ingroup object_index
    */
   typedef multi_index_container<
      account_object,
      indexed_by<
         ordered_unique<
            tag<by_id>,
            member< object, object_id_type, &object::id >
         >,
         ordered_unique<
            tag<by_name>,
            member<account_object, string, &account_object::name>
         >,
         // Add index that will sort accounts by the number of reputation votes that they have.
         ordered_non_unique<
            tag<by_reputation_votes>,
            member<account_object, uint64_t, &account_object::reputation_votes_count >
         >,
         // Add index that will separate publishers from users.
         ordered_non_unique<
            tag<by_publishers>,
            member<account_object, bool, &account_object::is_a_publisher>
         >,
         // Index that will sort users by the number of listings they host as publishers.
         ordered_non_unique<
            tag<by_listings_count>,
            member<account_object, uint64_t, &account_object::listings_count>
         >,
         // Index that will sort publishers by their IP/domain address and allow quick search by address.
         // Publishers should not be allowed to have same IP/domain,
         // but many users will have empty "publisher_ip" member which is why "ordered_non_unique" is used.
         ordered_non_unique<
            tag<by_publisher_ip>,
            member<account_object, string, &account_object::publisher_ip>
         >,
         // Index that will sort users based on their hardware info.
         ordered_non_unique<
            tag<by_hardware_info>,
            composite_key<
                account_object,
                member<account_object, string, &account_object::drive_id>,
                member<account_object, string, &account_object::mac_address>
            >
         >
      >
   > account_multi_index_type;

   /**
    * @ingroup object_index
    */
   typedef generic_index<account_object, account_multi_index_type> account_index;

}}

FC_REFLECT( graphene::chain::account_object_name,
	(id)
	(name)
)

FC_REFLECT_DERIVED( graphene::chain::account_object,
                    (graphene::db::object),
                    (registrar)(referrer)
                    (network_fee_percentage)(referrer_rewards_percentage)
                    (name)(owner)(active)(options)(statistics)(whitelisting_accounts)(blacklisting_accounts)
                    (whitelisted_accounts)(blacklisted_accounts)
                    (cashback_vb)
                    (escrow_vb)
                    (publisher_vb)
                    (founder_sale_vb)
                    (referrer_sale_vb)
                    (owner_special_authority)(active_special_authority)
                    (top_n_control_flags)
                    (allowed_assets)
                    (drive_id)
                    (mac_address)
                    (received_welcome_bonus)
                    (is_a_publisher)
                    (publisher_ip)
                    (is_an_escrow)
                    (escrow_fee)
                    (buyers)
                    (escrows)
                    (implicit_escrow_options)
                    (referral_score)
                    (listings_score)
                    (reputation_score)
                    (reputation_unweighted_score)
                    (trust_score)
                    (reliability_score)
                    (pop_score)
                    (reputation_votes_count)
                    (listings_count)
                    (sent_referral_bonus)
                    (is_referrer)
                    (publisher_fee)
                    (btc_address)
                    (eth_address)
                    (verified)
                    )

FC_REFLECT_DERIVED( graphene::chain::account_balance_object,
                    (graphene::db::object),
                    (owner)(asset_type)(balance) )

FC_REFLECT_DERIVED( graphene::chain::account_statistics_object,
                    (graphene::chain::object),
                    (owner)
                    (most_recent_op)
                    (total_ops)(removed_ops)
                    (total_core_in_orders)
                    (lifetime_fees_paid)
                    (pending_fees)(pending_vested_fees)
                    (reputation_votes)
                    (my_reputation_votes)
                  )


