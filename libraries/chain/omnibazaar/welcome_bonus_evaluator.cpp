#include <welcome_bonus_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/app/database_api.hpp>

namespace omnibazaar {

    graphene::chain::void_result welcome_bonus_evaluator::do_evaluate(const welcome_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));
            const graphene::chain::database& d = db();

            // Check if the bonus is depleted.
            bonus_ddump((d.get_dynamic_global_properties().welcome_bonus));
            FC_ASSERT(d.get_dynamic_global_properties().welcome_bonus < OMNIBAZAAR_WELCOME_BONUS_LIMIT, "Welcome Bonus is depleted.");

            // Check if account already received a bonus.
            const graphene::chain::account_object receiver = op.receiver(d);
            bonus_ddump((receiver));
            FC_ASSERT(!receiver.received_welcome_bonus, "${name} already received Welcome Bonus.", ("name", receiver.name));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::asset welcome_bonus_evaluator::do_apply(const welcome_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = d.get_welcome_bonus_amount();
            bonus_ddump((bonus_sum));

            // Send bonus.
            bonus_dlog("Adjusting balance.");
            d.adjust_balance(op.receiver, bonus_sum);

            // Set user hardware info to prevent multiple bonuses per machine.
            bonus_dlog("Setting hardware info.");
            d.modify(op.receiver(d), [&](graphene::chain::account_object& a) {
                a.received_welcome_bonus = true;
            });

            // Adjust asset supply value.
            bonus_dlog("Adjusting supply value.");
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            // Adjust the number of total issued bonus coins.
            bonus_dlog("Adjusting total bonus value.");
            d.modify(d.get_dynamic_global_properties(), [&bonus_sum](graphene::chain::dynamic_global_property_object& prop) {
               prop.welcome_bonus += bonus_sum;
            });

            return graphene::chain::asset(bonus_sum);
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }
}
