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
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/hardfork.hpp>
#include <fc/uint128.hpp>

#include "omnibazaar_util.hpp"

namespace graphene { namespace chain {

share_type cut_fee(share_type a, uint16_t p)
{
   if( a == 0 || p == 0 )
      return 0;
   if( p == GRAPHENE_100_PERCENT )
      return a;

   fc::uint128 r(a.value);
   r *= p;
   r /= GRAPHENE_100_PERCENT;
   return r.to_uint64();
}

void account_balance_object::adjust_balance(const asset& delta)
{
   assert(delta.asset_id == asset_type);
   balance += delta.amount;
}

void account_statistics_object::process_fees(const account_object& a, database& d) const
{
   if( pending_fees > 0 || pending_vested_fees > 0 )
   {
      auto pay_out_fees = [&](const account_object& account, share_type core_fee_total, bool require_vesting)
      {
         share_type network_cut = cut_fee(core_fee_total, account.network_fee_percentage);
         assert( network_cut <= core_fee_total );

#ifndef NDEBUG
         const auto& props = d.get_global_properties();

         share_type reserveed = cut_fee(network_cut, props.parameters.reserve_percent_of_fee);
         share_type accumulated = network_cut - reserveed;
         assert( accumulated + reserveed == network_cut );
#endif
         share_type referral = core_fee_total - network_cut;

         d.modify(asset_dynamic_data_id_type()(d), [network_cut](asset_dynamic_data_object& d) {
            d.accumulated_fees += network_cut;
         });

         // Potential optimization: Skip some of this math and object lookups by special casing on the account type.
         // For example, if the account is a lifetime member, we can skip all this and just deposit the referral to
         // it directly.
         share_type referrer_cut = cut_fee(referral, account.referrer_rewards_percentage);
         share_type registrar_cut = referral - referrer_cut;

         d.deposit_cashback(d.get(account.referrer), referrer_cut, require_vesting);
         d.deposit_cashback(d.get(account.registrar), registrar_cut, require_vesting);

         assert( referrer_cut + registrar_cut + accumulated + reserveed == core_fee_total );
      };

      pay_out_fees(a, pending_fees, true);
      pay_out_fees(a, pending_vested_fees, false);

      d.modify(*this, [&](account_statistics_object& s) {
         s.lifetime_fees_paid += pending_fees + pending_vested_fees;
         s.pending_fees = 0;
         s.pending_vested_fees = 0;
      });
   }
}

void account_statistics_object::pay_fee( share_type core_fee, share_type cashback_vesting_threshold )
{
   if( core_fee > cashback_vesting_threshold )
      pending_fees += core_fee;
   else
      pending_vested_fees += core_fee;
}

set<account_id_type> account_member_index::get_account_members(const account_object& a)const
{
   set<account_id_type> result;
   for( auto auth : a.owner.account_auths )
      result.insert(auth.first);
   for( auto auth : a.active.account_auths )
      result.insert(auth.first);
   return result;
}
set<public_key_type> account_member_index::get_key_members(const account_object& a)const
{
   set<public_key_type> result;
   for( auto auth : a.owner.key_auths )
      result.insert(auth.first);
   for( auto auth : a.active.key_auths )
      result.insert(auth.first);
   result.insert( a.options.memo_key );
   return result;
}
set<address> account_member_index::get_address_members(const account_object& a)const
{
   set<address> result;
   for( auto auth : a.owner.address_auths )
      result.insert(auth.first);
   for( auto auth : a.active.address_auths )
      result.insert(auth.first);
   result.insert( a.options.memo_key );
   return result;
}

void account_member_index::object_inserted(const object& obj)
{
    assert( dynamic_cast<const account_object*>(&obj) ); // for debug only
    const account_object& a = static_cast<const account_object&>(obj);

    auto account_members = get_account_members(a);
    for( auto item : account_members )
       account_to_account_memberships[item].insert(obj.id);

    auto key_members = get_key_members(a);
    for( auto item : key_members )
       account_to_key_memberships[item].insert(obj.id);

    auto address_members = get_address_members(a);
    for( auto item : address_members )
       account_to_address_memberships[item].insert(obj.id);
}

void account_member_index::object_removed(const object& obj)
{
    assert( dynamic_cast<const account_object*>(&obj) ); // for debug only
    const account_object& a = static_cast<const account_object&>(obj);

    auto key_members = get_key_members(a);
    for( auto item : key_members )
       account_to_key_memberships[item].erase( obj.id );

    auto address_members = get_address_members(a);
    for( auto item : address_members )
       account_to_address_memberships[item].erase( obj.id );

    auto account_members = get_account_members(a);
    for( auto item : account_members )
       account_to_account_memberships[item].erase( obj.id );
}

void account_member_index::about_to_modify(const object& before)
{
   before_key_members.clear();
   before_account_members.clear();
   assert( dynamic_cast<const account_object*>(&before) ); // for debug only
   const account_object& a = static_cast<const account_object&>(before);
   before_key_members     = get_key_members(a);
   before_address_members = get_address_members(a);
   before_account_members = get_account_members(a);
}

void account_member_index::object_modified(const object& after)
{
    assert( dynamic_cast<const account_object*>(&after) ); // for debug only
    const account_object& a = static_cast<const account_object&>(after);

    {
       set<account_id_type> after_account_members = get_account_members(a);
       vector<account_id_type> removed; removed.reserve(before_account_members.size());
       std::set_difference(before_account_members.begin(), before_account_members.end(),
                           after_account_members.begin(), after_account_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_account_memberships[*itr].erase(after.id);

       vector<object_id_type> added; added.reserve(after_account_members.size());
       std::set_difference(after_account_members.begin(), after_account_members.end(),
                           before_account_members.begin(), before_account_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_account_memberships[*itr].insert(after.id);
    }


    {
       set<public_key_type> after_key_members = get_key_members(a);

       vector<public_key_type> removed; removed.reserve(before_key_members.size());
       std::set_difference(before_key_members.begin(), before_key_members.end(),
                           after_key_members.begin(), after_key_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_key_memberships[*itr].erase(after.id);

       vector<public_key_type> added; added.reserve(after_key_members.size());
       std::set_difference(after_key_members.begin(), after_key_members.end(),
                           before_key_members.begin(), before_key_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_key_memberships[*itr].insert(after.id);
    }

    {
       set<address> after_address_members = get_address_members(a);

       vector<address> removed; removed.reserve(before_address_members.size());
       std::set_difference(before_address_members.begin(), before_address_members.end(),
                           after_address_members.begin(), after_address_members.end(),
                           std::inserter(removed, removed.end()));

       for( auto itr = removed.begin(); itr != removed.end(); ++itr )
          account_to_address_memberships[*itr].erase(after.id);

       vector<address> added; added.reserve(after_address_members.size());
       std::set_difference(after_address_members.begin(), after_address_members.end(),
                           before_address_members.begin(), before_address_members.end(),
                           std::inserter(added, added.end()));

       for( auto itr = added.begin(); itr != added.end(); ++itr )
          account_to_address_memberships[*itr].insert(after.id);
    }

}

void account_referrer_index::object_inserted( const object& obj )
{
    const account_object& a = static_cast<const account_object&>(obj);
    referred_by[a.referrer].insert(a.get_id());
}
void account_referrer_index::object_removed( const object& obj )
{
    const account_object& a = static_cast<const account_object&>(obj);
    const auto& iter = referred_by.find(a.referrer);
    if(iter == referred_by.cend())
        return;

    iter->second.erase(a.get_id());
    if(iter->second.size() <= 0)
    {
        referred_by.erase(iter);
    }
}
void account_referrer_index::about_to_modify( const object& before )
{
    object_removed(before);
}
void account_referrer_index::object_modified( const object& after  )
{
    object_inserted(after);
}

void account_escrow_index::object_inserted( const object& obj )
{
    const account_object& a = static_cast<const account_object&>(obj);
    if(a.is_an_escrow)
    {
		account_object_name newObj(a.get_id(), a.name);
		this->insert_keeping_sorted(newObj);
    }
}

void account_escrow_index::object_removed( const object& obj )
{
    const account_object& a = static_cast<const account_object&>(obj);
   
	auto escrow_it = std::lower_bound(current_escrows.begin(), current_escrows.end(), a.name, account_object_name_comparer());

	if (escrow_it != current_escrows.end() && escrow_it->id == a.get_id())
	{
		current_escrows.erase(escrow_it);
	}
}

void account_escrow_index::about_to_modify( const object& before )
{
}

void account_escrow_index::object_modified( const object& after  )
{
    const account_object& a = static_cast<const account_object&>(after);
	
    if(a.is_an_escrow)
    {
		// check whether to add a new account_object_name or just to update the existing one
		auto escrow_it = std::lower_bound(current_escrows.begin(), current_escrows.end(), a.name, account_object_name_comparer());

		if (escrow_it == current_escrows.end() || escrow_it->id != a.get_id())
		{
			// this means add
			account_object_name newObj(a.get_id(), a.name);
			current_escrows.insert(escrow_it, newObj);
		}
		else
		{
			// this means update
			escrow_it->name = a.name;
		}		
    }
    else
    {
		this->object_removed(a);
    }
}


void account_escrow_index::insert_keeping_sorted(const account_object_name& account_object_name)
{
	auto escrow_it = std::lower_bound(current_escrows.begin(), current_escrows.end(), account_object_name.name, account_object_name_comparer());
	current_escrows.insert(escrow_it, account_object_name);
}

std::vector<account_object_name> account_escrow_index::filter_by_name(uint32_t start, uint32_t limit, const std::string& search_term) const
{
	std::vector<account_object_name> result;
	result.reserve(limit);

	// find the first match for the search_term
	auto escrow_it = std::lower_bound(current_escrows.begin(), current_escrows.end(), search_term, account_object_name_comparer()); 

	// check if we can advance the iterator 'start' times
	if (escrow_it - current_escrows.begin() + start >= current_escrows.size())
	{
		return result;
	}

	// advance the iterator 'start' times
	escrow_it += start;
	
	while (escrow_it != current_escrows.end() && result.size() < limit)
	{
		// get the current escrow name
		const std::string current_escrow_name = escrow_it->name;
		
		// if the escrow name doesn't start with search term, we're done
		if (current_escrow_name.find(search_term) != 0)
			break;

		result.push_back(*escrow_it);
		escrow_it++;
	}

	return result;
}

void account_object::update_reputation(database& db, const account_id_type target, const account_id_type from, const uint16_t reputation, const asset amount)
{
    pop_dlog("Updating reputation vote ${vote} for ${seller} from ${buyer}.",
             ("vote", reputation)("seller", target)("buyer", from));

    // Temporarily update reputation_votes to calculate Reputation Score for display.
    account_object target_account = target(db);
    account_statistics_object stats = target_account.statistics(db);
    if(reputation == OMNIBAZAAR_REPUTATION_DEFAULT)
        stats.reputation_votes.erase(from);
    else
        stats.reputation_votes[from] = std::make_pair(reputation, amount);

    // Calculate score outside of database::modify callback.
    fc::uint128_t weighted_votes_sum = 0;
    fc::uint128_t weight_sum = 0;
    for(const auto& iter : stats.reputation_votes)
    {
        const std::pair<uint16_t, asset> vote_info = iter.second;
        weighted_votes_sum += fc::uint128_t(vote_info.first) * vote_info.second.amount.value;
        weight_sum += vote_info.second.amount.value;
    }
    // Just for display, store score without transfers number weight applied.
    // Raw formula is:
    //    weighted_votes_sum
    //    ------------------
    //        weight_sum         * GRAPHENE_100_PERCENT
    // -------------------------
    // OMNIBAZAAR_REPUTATION_MAX
    const uint16_t score = weight_sum > 0
            ? (weighted_votes_sum * GRAPHENE_100_PERCENT / weight_sum / OMNIBAZAAR_REPUTATION_MAX).to_integer()
            : 0;
    pop_ddump((score));

    // Update reputation for receiving account.
    db.modify(target(db).statistics(db), [&](account_statistics_object& s){
        if(reputation == OMNIBAZAAR_REPUTATION_DEFAULT)
        {
            // Default reputation votes do not count towards Reputation Score
            // so there's no point in storing them and wasting space.
            s.reputation_votes.erase(from);
        }
        else
        {
            s.reputation_votes[from] = std::make_pair(reputation, amount);
        }
    });
    db.modify(target(db), [&](account_object &acc){
        acc.reputation_unweighted_score = score;
        acc.reputation_votes_count = stats.reputation_votes.size();
    });

    // Update reputation for sending account.
    db.modify(from(db).statistics(db), [&](account_statistics_object& s){
        if(reputation > OMNIBAZAAR_REPUTATION_DEFAULT)
        {
            s.my_reputation_votes.insert(target);
        }
        else
        {
            s.my_reputation_votes.erase(target);
        }
    });
}

} } // graphene::chain
