#include <witness_bonus_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/operation_history_object.hpp>

namespace omnibazaar {

    graphene::chain::void_result witness_bonus_evaluator::do_evaluate( const witness_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            const graphene::chain::database& d = db();

            // Check that bonus is not depleted yet.
            bonus_ddump((d.get_dynamic_global_properties().witness_bonus)(OMNIBAZAAR_WITNESS_BONUS_TOTAL_COINS));
            FC_ASSERT(d.get_dynamic_global_properties().witness_bonus < OMNIBAZAAR_WITNESS_BONUS_TOTAL_COINS,
                      "Witness Bonus is depleted.");

            // Check that bonus receiver is valid.
            const auto& idx = d.get_index_type<graphene::chain::witness_index>().indices().get<graphene::chain::by_account>();
            const auto witness_itr = idx.find(op.receiver);
            FC_ASSERT(witness_itr != idx.end(), "${a} is not a witness.", ("a", op.receiver(d).name));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::asset witness_bonus_evaluator::do_apply( const witness_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = get_bonus_sum();
            bonus_ddump((bonus_sum));

            // Send bonus.
            bonus_dlog("Adjusting balance.");
            d.adjust_balance(op.receiver, bonus_sum);

            // Adjust asset supply value.
            bonus_dlog("Adjusting supply value.");
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            // Adjust the number of total issued bonus coins.
            bonus_dlog("Adjusting total bonus value.");
            d.modify(d.get_dynamic_global_properties(), [&bonus_sum](graphene::chain::dynamic_global_property_object& prop) {
               prop.witness_bonus += bonus_sum;
            });

            return graphene::chain::asset(bonus_sum);
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::share_type witness_bonus_evaluator::get_bonus_sum()const
    {
        bonus_ddump((""));

        const graphene::chain::database& d = db();

        bonus_ddump((d.get_chain_properties().initial_timestamp)(d.head_block_time()));
        const fc::microseconds time_since_genesis = d.head_block_time() - d.get_chain_properties().initial_timestamp;
        bonus_ddump((time_since_genesis));

        // First distribution interval.
        if(time_since_genesis.to_seconds() <= OMNIBAZAAR_WITNESS_BONUS_TIME_LIMIT_1)
        {
            return OMNIBAZAAR_WITNESS_BONUS_COINS_PER_SECOND_1 * d.block_interval();
        }
        // Second distribution interval.
        else if(time_since_genesis.to_seconds() <= (OMNIBAZAAR_WITNESS_BONUS_TIME_LIMIT_1 + OMNIBAZAAR_WITNESS_BONUS_TIME_LIMIT_2))
        {
            return OMNIBAZAAR_WITNESS_BONUS_COINS_PER_SECOND_2 * d.block_interval();
        }
        // Until bonus is depleted.
        else
        {
            return std::min(int64_t(OMNIBAZAAR_WITNESS_BONUS_COINS_PER_SECOND_3 * d.block_interval()),
                            int64_t(OMNIBAZAAR_WITNESS_BONUS_TOTAL_COINS - d.get_dynamic_global_properties().witness_bonus.value));
        }
    }

}
