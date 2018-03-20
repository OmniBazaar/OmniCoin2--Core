#include <welcome_bonus_evaluator.hpp>
#include <graphene/app/database_api.hpp>

namespace omnibazaar {

    graphene::chain::void_result welcome_bonus_evaluator::do_evaluate(const welcome_bonus_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Check if the bonus is depleted.
            if(d.get_dynamic_global_properties().welcome_bonus >= (6.2e9 * GRAPHENE_BLOCKCHAIN_PRECISION))
            {
                FC_THROW("Welcome Bonus is depleted");
            }

            // Check if account already received a bonus.
            const auto receiver = graphene::app::database_api(d).get_account_by_name(op.receiver_name);
            if(!receiver.valid())
            {
                FC_THROW("Account '${name}' doesn't exist", ("name", op.receiver_name));
            }
			else
			{
                _receiver_account = (*receiver).get_id();
			}
            if (!d.is_welcome_bonus_available(op.drive_id, op.mac_address))
            {
                FC_THROW("Account '${name}' has already received a welcome bonus", ("name", receiver->name));
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result welcome_bonus_evaluator::do_apply(const welcome_bonus_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = get_bonus_sum() * GRAPHENE_BLOCKCHAIN_PRECISION;

            // Send bonus.
            d.adjust_balance(_receiver_account, bonus_sum);

            // Set user hardware info to prevent multiple bonuses per machine.
            d.modify(_receiver_account(d), [&](graphene::chain::account_object& a) {
                a.drive_id = op.drive_id;
                a.mac_address = op.mac_address;
            });

            // Adjust asset supply value.
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            // Adjust the number of total issued bonus coins.
            d.modify(d.get_dynamic_global_properties(), [&bonus_sum](graphene::chain::dynamic_global_property_object& prop) {
               prop.welcome_bonus += bonus_sum;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    double welcome_bonus_evaluator::get_bonus_sum()const
    {
        const auto users_count = graphene::app::database_api(db()).get_account_count();

        if      (users_count <= 1000   )    return 10000.;
        else if (users_count <= 10000  )    return 5000.;
        else if (users_count <= 100000 )    return 2500.;
        else if (users_count <= 1000000)    return 1250.;
        else                                return 625.;
    }

}
