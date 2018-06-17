#include <referral_bonus_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/app/database_api.hpp>

namespace omnibazaar {

    graphene::chain::void_result referral_bonus_evaluator::do_evaluate( const referral_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));
            const graphene::chain::database& d = db();

            // Check if the bonus is depleted.
            bonus_ddump((d.get_dynamic_global_properties().referral_bonus)(OMNIBAZAAR_REFERRAL_BONUS_LIMIT));
            FC_ASSERT(d.get_dynamic_global_properties().referral_bonus < OMNIBAZAAR_REFERRAL_BONUS_LIMIT, "Referral Bonus is depleted.");

            const graphene::chain::account_object referred = op.referred_account(d);
            FC_ASSERT(!referred.sent_referral_bonus, "${name} already sent a Referral Bonus.", ("name", referred.name));
            FC_ASSERT(referred.referrer == op.referrer_account, "Invalid referrer, can't send Referral Bonus.");
            FC_ASSERT(referred.received_welcome_bonus,
                      "${name} did not receive Welcome Bonus, can't send Referral Bonus.",
                      ("name", referred.name));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::asset referral_bonus_evaluator::do_apply( const referral_bonus_operation& op )
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
            d.adjust_balance(op.referrer_account, bonus_sum);

            // Set bonus flag.
            bonus_dlog("Settings bonus flag for referred account.");
            d.modify(op.referred_account(d), [](graphene::chain::account_object& a){
                a.sent_referral_bonus = true;
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
               prop.referral_bonus += bonus_sum;
            });

            return graphene::chain::asset(bonus_sum);
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::share_type referral_bonus_evaluator::get_bonus_sum()const
    {
        bonus_ddump((""));

        const auto users_count = graphene::app::database_api(db()).get_account_count();
        bonus_ddump((users_count));

        if      (users_count <= 10000  )    return 2500 * GRAPHENE_BLOCKCHAIN_PRECISION;
        else if (users_count <= 100000 )    return 1250 * GRAPHENE_BLOCKCHAIN_PRECISION;
        else if (users_count <= 1000000)    return 625  * GRAPHENE_BLOCKCHAIN_PRECISION;
        else								return 312  * GRAPHENE_BLOCKCHAIN_PRECISION + 5 * GRAPHENE_BLOCKCHAIN_PRECISION / 10;
    }

}
