#include <witness_bonus_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result witness_bonus_evaluator::do_evaluate( const witness_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            bonus_ddump((db().get_dynamic_global_properties().head_block_number)(OMNIBAZAAR_WITNESS_BLOCK_LIMIT));
            if(db().get_dynamic_global_properties().head_block_number > OMNIBAZAAR_WITNESS_BLOCK_LIMIT)
            {
                FC_THROW("Witness Bonus is depleted");
            }

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

            return graphene::chain::asset(bonus_sum);
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::share_type witness_bonus_evaluator::get_bonus_sum()const
    {
        bonus_ddump((""));

        const auto blocks_count = db().get_dynamic_global_properties().head_block_number;
        bonus_ddump((blocks_count));

        if      (blocks_count <= 25228800) return 200 * GRAPHENE_BLOCKCHAIN_PRECISION;
        else if (blocks_count <= 37843200) return 100 * GRAPHENE_BLOCKCHAIN_PRECISION;
        else                               return 50  * GRAPHENE_BLOCKCHAIN_PRECISION;
    }

}
