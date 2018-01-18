#include <graphene/chain/protocol/welcome_bonus.hpp>

namespace omnibazaar {

    void welcome_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( !receiver_name.empty() );
        FC_ASSERT( !drive_id.empty() );
        FC_ASSERT( !mac_address.empty() );
    }

    graphene::chain::share_type welcome_bonus_operation::calculate_fee(const fee_parameters_type& k)const
    {
        return k.fee + calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
    }

}
