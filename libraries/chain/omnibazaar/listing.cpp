#include <listing.hpp>
#include <../omnibazaar/listing_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/hardfork.hpp>

namespace omnibazaar {

    void listing_create_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
        FC_ASSERT( price.amount > 0 );
        // Vested balances work only with core asset, so for now implement support only for core asset in listing operations.
        FC_ASSERT( price.asset_id == graphene::chain::asset_id_type(), "Listings support only ${c} currency.", ("c", GRAPHENE_SYMBOL) );
    }

    omnibazaar_fee_type listing_create_operation::calculate_omnibazaar_fee(const graphene::chain::database &db)const
    {
        omnibazaar_fee_type fees;
        // Add publisher fee if seller is not hosting this listing himself.
        if(seller != publisher)
        {
            graphene::chain::share_type publisher_fee = graphene::chain::cut_fee(price.amount, publisher(db).publisher_fee);
            // If OM-749 is in effect - limit publisher fee value according to chain properties.
            if(db.get_global_properties().parameters.extensions.value.publisher_fee_min.valid())
            {
                publisher_fee = std::max(publisher_fee, *db.get_global_properties().parameters.extensions.value.publisher_fee_min);
            }
            if(db.get_global_properties().parameters.extensions.value.publisher_fee_max.valid())
            {
                publisher_fee = std::min(publisher_fee, *db.get_global_properties().parameters.extensions.value.publisher_fee_max);
            }

            if(publisher_fee > 0)
            {
                fees.publisher_fee = graphene::chain::asset(publisher_fee, price.asset_id);
            }
        }
        return fees;
    }

    void listing_update_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
        if(price.valid())
        {
            // Vested balances work only with core asset, so for now implement support only for core asset in listing operations.
            FC_ASSERT( price->asset_id == graphene::chain::asset_id_type(), "Listings support only ${c} currency.", ("c", GRAPHENE_SYMBOL) );
        }

        const bool has_action = publisher.valid()
                || price.valid()
                || listing_hash.valid()
                || quantity.valid()
                || update_expiration_time
                || priority_fee.valid();
        FC_ASSERT( has_action );
    }

    omnibazaar_fee_type listing_update_operation::calculate_omnibazaar_fee(const graphene::chain::database &db)const
    {
        omnibazaar_fee_type fees;
        const graphene::chain::account_id_type final_publisher = publisher.valid() ? *publisher : listing_id(db).publisher;
        // If listing will be moved to or will stay on seller's server, then no extra fees are applied.
        if(final_publisher == seller)
        {
            // No fees.
        }
        // If listing will be moved to another publisher or stay on existing publisher but with extended expiration,
        // then add publisher fee.
        else if(publisher.valid() || update_expiration_time)
        {
            const graphene::chain::asset final_price = price.valid() ? *price : listing_id(db).price;
            graphene::chain::share_type publisher_fee = graphene::chain::cut_fee(final_price.amount, final_publisher(db).publisher_fee);
            // If OM-749 is in effect - limit publisher fee value according to chain properties.
            if(db.get_global_properties().parameters.extensions.value.publisher_fee_min.valid())
            {
                publisher_fee = std::max(publisher_fee, *db.get_global_properties().parameters.extensions.value.publisher_fee_min);
            }
            if(db.get_global_properties().parameters.extensions.value.publisher_fee_max.valid())
            {
                publisher_fee = std::min(publisher_fee, *db.get_global_properties().parameters.extensions.value.publisher_fee_max);
            }

            if(publisher_fee > 0)
            {
                fees.publisher_fee = graphene::chain::asset(publisher_fee, final_price.asset_id);
            }
        }
        return fees;
    }

    void listing_delete_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
    }

    void listing_report_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
    }
}
