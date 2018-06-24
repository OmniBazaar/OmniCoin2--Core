#include <listing.hpp>

namespace omnibazaar {

    void listing_create_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
        FC_ASSERT( price.amount > 0 );
        // Vested balances work only with core asset, so for now implement support only for core asset in listing operations.
        FC_ASSERT( price.asset_id == graphene::chain::asset_id_type(), "Listings support only ${c} currency.", ("c", GRAPHENE_SYMBOL) );
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
                || update_expiration_time;
        FC_ASSERT( has_action );
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
