#include <witness_bonus_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result witness_bonus_evaluator::do_evaluate( const witness_bonus_operation& op )
    {
        try
        {
            if(db().get_dynamic_global_properties().head_block_number > OMNIBAZAAR_WITNESS_BLOCK_LIMIT)
            {
                FC_THROW("Witness Bonus is depleted");
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result witness_bonus_evaluator::do_apply( const witness_bonus_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = get_bonus_sum() * GRAPHENE_BLOCKCHAIN_PRECISION;

            // Send bonus.
            d.adjust_balance(op.receiver, bonus_sum);

            // Adjust asset supply value.
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    double witness_bonus_evaluator::get_bonus_sum()const
    {
        const auto blocks_count = db().get_dynamic_global_properties().head_block_number;

        if      (blocks_count <= 25228800) return 200.;
        else if (blocks_count <= 37843200) return 100.;
        else                               return 50.;
    }

}
