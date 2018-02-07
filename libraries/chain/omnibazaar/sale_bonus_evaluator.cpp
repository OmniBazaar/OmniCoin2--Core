#include <sale_bonus_evaluator.hpp>
#include <graphene/app/database_api.hpp>

namespace omnibazaar {

    graphene::chain::void_result sale_bonus_evaluator::do_evaluate( const sale_bonus_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();

            // Check if the bonus is depleted.
            if(d.get_dynamic_global_properties().sale_bonus >= OMNIBAZAAR_SALE_BONUS_LIMIT)
            {
                FC_THROW("Sale Bonus is depleted");
            }

            const graphene::chain::account_object& seller = op.seller(d);
            if(seller.buyers.find(op.buyer) != seller.buyers.end())
            {
                FC_THROW("Sale bonus already received for seller '${seller}' and buyer '${buyer}'",
                         ("seller", seller.name)
                         ("buyer", op.buyer(d).name));
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result sale_bonus_evaluator::do_apply( const sale_bonus_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Calculate available bonus value.
            const graphene::chain::share_type bonus_sum = get_bonus_sum() * GRAPHENE_BLOCKCHAIN_PRECISION;

            // Send bonus.
            d.adjust_balance(op.seller, bonus_sum);

            // Add buyer to the list to prevent multiple bonuses per account.
            d.modify(op.seller(d), [&op](graphene::chain::account_object& a) {
                a.buyers.insert(op.buyer);
            });

            // Adjust asset supply value.
            const graphene::chain::asset_object& asset = d.get_core_asset();
            d.modify(asset.dynamic_asset_data_id(d), [&bonus_sum](graphene::chain::asset_dynamic_data_object& dynamic_asset){
                dynamic_asset.current_supply += bonus_sum;
            });

            // Adjust the number of total issued bonus coins.
            d.modify(d.get_dynamic_global_properties(), [&bonus_sum](graphene::chain::dynamic_global_property_object& prop) {
               prop.sale_bonus += bonus_sum;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    double sale_bonus_evaluator::get_bonus_sum()const
    {
        const auto users_count = graphene::app::database_api(db()).get_account_count();

        if      (users_count <= 100000  )   return 500.;
        else if (users_count <= 1000000 )   return 250.;
        else if (users_count <= 10000000)   return 125.;
        else                                return 62.5;
    }

}
