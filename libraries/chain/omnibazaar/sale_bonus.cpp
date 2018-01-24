#include <sale_bonus.hpp>

namespace omnibazaar {

    void sale_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( seller != buyer );
    }

    graphene::chain::share_type sale_bonus_operation::calculate_fee(const fee_parameters_type& k)const
    {
        return k.fee + calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
    }

}
