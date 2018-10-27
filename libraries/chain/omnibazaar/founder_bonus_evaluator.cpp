#include <founder_bonus_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result founder_bonus_evaluator::do_evaluate( const founder_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            const auto& dyn_prop = db().get_dynamic_global_properties();
            bonus_ddump((dyn_prop.head_block_number)(dyn_prop.founder_bonus)(OMNIBAZAAR_FOUNDER_BONUS_TIME_LIMIT)(OMNIBAZAAR_FOUNDER_BONUS_COINS_LIMIT));
            FC_ASSERT(dyn_prop.founder_bonus < OMNIBAZAAR_FOUNDER_BONUS_COINS_LIMIT, "Developer Bonus is depleted.");

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::asset founder_bonus_evaluator::do_apply( const founder_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            graphene::chain::database& d = db();

            // Block interval can change and that will change bonus amounts per block,
            // and amounts will no longer map to bonus distribution timeline evenly (without a remainder).
            // Because of this, the very last bonus portion can be less than just "coins_per_second * block_time".
            const graphene::chain::share_type bonus_sum = std::min(
                        // Usual bonus amount.
                        OMNIBAZAAR_FOUNDER_BONUS_COINS_PER_SECOND * d.block_interval(),
                        // Whatever is left in bonus reserves.
                        OMNIBAZAAR_FOUNDER_BONUS_COINS_LIMIT - d.get_dynamic_global_properties().founder_bonus.value);
            bonus_ddump((bonus_sum));

            // Send bonus.
            bonus_dlog("Adjusting balance.");
            d.adjust_balance(OMNIBAZAAR_FOUNDER_ACCOUNT, bonus_sum);

            // Adjust asset supply value.
            bonus_dlog("Adjusting supply value.");
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            // Adjust the number of total issued bonus coins.
            bonus_dlog("Adjusting total bonus value.");
            d.modify(d.get_dynamic_global_properties(), [&bonus_sum](graphene::chain::dynamic_global_property_object& prop) {
               prop.founder_bonus += bonus_sum;
            });

            return graphene::chain::asset(bonus_sum);
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}
