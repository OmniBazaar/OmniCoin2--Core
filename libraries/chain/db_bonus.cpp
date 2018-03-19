#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

bool database::is_welcome_bonus_available(const string &harddrive_id, const string &mac_address)const
{
    if (harddrive_id.empty() || mac_address.empty())
    {
        wlog("We can't get your machine identity. "
             "Your new user account has been created, but that new account will not receive a Welcome Bonus.");
        return false;
    }

    const auto& account_idx = dynamic_cast<const primary_index<account_index>&>(get_index_type<account_index>());
    const auto& bonus_idx = account_idx.get_secondary_index<account_welcome_bonus_index>();
    const bool has_drive_id = bonus_idx.drive_ids.find(harddrive_id) != bonus_idx.drive_ids.end();
    const bool has_mac_address = bonus_idx.mac_addresses.find(mac_address) != bonus_idx.mac_addresses.end();
    if(has_drive_id || has_mac_address)
    {
        wlog("You have already received a sign-up bonus for a user on this machine. "
             "Your new user account has been created, but that new account will not receive a Welcome Bonus.");
        return false;
    }

    return true;
}

bool database::is_referral_bonus_available()const
{
    return get_dynamic_global_properties().referral_bonus < OMNIBAZAAR_REFERRAL_BONUS_LIMIT;
}

bool database::is_sale_bonus_available(const account_id_type &seller_id, const account_id_type &buyer_id)const
{
    if(get_dynamic_global_properties().sale_bonus >= OMNIBAZAAR_SALE_BONUS_LIMIT)
    {
        ilog("Sale Bonus depleted");
        return false;
    }

    const account_object seller = seller_id(*this);
    if(seller.buyers.find(buyer_id) != seller.buyers.end())
    {
        ilog("Sale bonus already received for seller '${seller}' and buyer '${buyer}'",
             ("seller", seller.name)
             ("buyer", buyer_id(*this).name));
        return false;
    }

    return true;
}

}}
