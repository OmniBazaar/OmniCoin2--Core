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

            // Check if the bonus is depleted.
            bonus_ddump((db().get_dynamic_global_properties().referral_bonus)(OMNIBAZAAR_REFERRAL_BONUS_LIMIT));
            if(db().get_dynamic_global_properties().referral_bonus >= OMNIBAZAAR_REFERRAL_BONUS_LIMIT)
            {
                FC_THROW("Referral Bonus is depleted");
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result referral_bonus_evaluator::do_apply( const referral_bonus_operation& op )
    {
        try
        {
            bonus_ddump((op));

            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = get_bonus_sum() * GRAPHENE_BLOCKCHAIN_PRECISION;
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
               prop.referral_bonus += bonus_sum;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    double referral_bonus_evaluator::get_bonus_sum()const
    {
        bonus_ddump((""));

        const auto users_count = graphene::app::database_api(db()).get_account_count();
        bonus_ddump((users_count));

        if      (users_count <= 10000  )    return 2500.;
        else if (users_count <= 100000 )    return 1250.;
        else if (users_count <= 1000000)    return 625.;
        else								return 312.5;
    }

}
