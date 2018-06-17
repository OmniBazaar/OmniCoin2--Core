#include <graphene/chain/database.hpp>
#include <graphene/chain/witness_object.hpp>

namespace graphene { namespace chain {

set<account_id_type> database::get_implicit_escrows(const account_id_type target_account_id) const
{
    set<account_id_type> result;

    const account_object target_account = target_account_id(*this);

    // Add accounts that are current active witnesses.
    if(target_account.implicit_escrow_options.active_witness)
    {
        const auto& active_witnesses = get_global_properties().active_witnesses;
        for(const auto witness_id : active_witnesses)
        {
            const witness_object witness_obj = witness_id(*this);
            if(witness_obj.witness_account(*this).is_an_escrow)
            {
                result.insert(witness_obj.witness_account);
            }
        }
    }

    // Add witnesses that target account voted for.
    if(target_account.implicit_escrow_options.voted_witness)
    {
        const auto& witness_by_vote_idx = get_index_type<witness_index>().indices().get<by_vote_id>();
        for(const vote_id_type vote_id : target_account.options.votes)
        {
            const auto witness_iter = witness_by_vote_idx.find(vote_id);
            if(witness_iter == witness_by_vote_idx.end())
                continue;

            if(witness_iter->witness_account(*this).is_an_escrow)
            {
                result.insert(witness_iter->witness_account);
            }
        }
    }

    // Add accounts that target account gave positive rating to.
    if(target_account.implicit_escrow_options.positive_rating)
    {
        const account_statistics_object& stats = target_account.statistics(*this);
        for(const auto account_id : stats.my_reputation_votes)
        {
            if(account_id(*this).is_an_escrow)
            {
                result.insert(account_id);
            }
        }
    }

    return result;
}

}}
