#include <graphene/chain/database.hpp>
#include <omnibazaar_util.hpp>

namespace graphene { namespace chain {

bool database::is_welcome_bonus_available(const string &harddrive_id, const string &mac_address)const
{
    if (harddrive_id.empty() || mac_address.empty())
    {
        bonus_wlog("We can't get your machine identity. "
                   "Your new user account has been created, but that new account will not receive a Welcome Bonus.");
        return false;
    }

    const auto& hardware_idx = get_index_type<account_index>().indices().get<by_hardware_info>();
    auto iters = hardware_idx.equal_range(std::make_tuple(harddrive_id, mac_address));
    while(iters.first != iters.second)
    {
        if(iters.first->received_welcome_bonus)
        {
            bonus_wlog("You have already received a sign-up bonus for a user on this machine. "
                       "Your new user account has been created, but that new account will not receive a Welcome Bonus.");
            return false;
        }
        ++iters.first;
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
