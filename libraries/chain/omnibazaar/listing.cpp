#include <listing.hpp>

namespace omnibazaar {

    void listing_create_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
    }

    graphene::chain::share_type listing_create_operation::calculate_fee(const fee_parameters_type& k)const
    {
        return k.fee + calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
    }
}
