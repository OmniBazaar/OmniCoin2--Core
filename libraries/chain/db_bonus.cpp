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

    // Scan blockchain to check if these HDD ID and MAC were already registered.
    for(uint32_t block_num = 1, total_blocks = get_dynamic_global_properties().head_block_number; block_num <= total_blocks; ++block_num)
    {
        const fc::optional<signed_block> block = fetch_block_by_number(block_num);
        if(block.valid())
        {
            for(const processed_transaction& tx : block->transactions)
            {
                for(const operation& op : tx.operations)
                {
                    if(op.which() == operation::tag<omnibazaar::welcome_bonus_operation>::value)
                    {
                        const omnibazaar::welcome_bonus_operation& welcome_op = op.get<omnibazaar::welcome_bonus_operation>();
                        if (welcome_op.drive_id == harddrive_id || welcome_op.mac_address == mac_address)
                        {
                            wlog("You have already received a sign-up bonus for a user on this machine. "
                                 "Your new user account has been created, but that new account will not receive a Welcome Bonus.");
                            return false;
                        }
                    }
                }
            }
        }
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