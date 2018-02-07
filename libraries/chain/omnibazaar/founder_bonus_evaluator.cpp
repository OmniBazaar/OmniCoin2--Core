#include <founder_bonus_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result founder_bonus_evaluator::do_evaluate( const founder_bonus_operation& op )
    {
        try
        {
            if(db().get_dynamic_global_properties().head_block_number > OMNIBAZAAR_FOUNDER_BLOCK_LIMIT)
            {
                FC_THROW("Founder Bonus is depleted");
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result founder_bonus_evaluator::do_apply( const founder_bonus_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            const graphene::chain::share_type bonus_sum = 200 * GRAPHENE_BLOCKCHAIN_PRECISION;

            // Send bonus.
            d.adjust_balance(OMNIBAZAAR_FOUNDER_ACCOUNT, bonus_sum);

            // Adjust asset supply value.
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}
