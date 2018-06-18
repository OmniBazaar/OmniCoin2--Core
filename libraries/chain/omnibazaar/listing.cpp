#include <listing.hpp>

namespace omnibazaar {

    void listing_create_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
        FC_ASSERT( price.amount > 0 );
    }

    void listing_update_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );

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
}
