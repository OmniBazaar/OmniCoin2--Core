#include <exchange.hpp>

namespace omnibazaar {

    void exchange_create_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( coin_name.length(), "Coin name is empty." );
        FC_ASSERT( tx_id.length(), "Transaction ID is empty." );
    }

    graphene::chain::share_type exchange_create_operation::calculate_fee(const fee_parameters_type& k)const
    {
        return k.fee + calculate_data_fee( fc::raw::pack_size(coin_name) + fc::raw::pack_size(tx_id), k.price_per_kbyte );
    }

    void exchange_complete_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( amount.amount > 0 );
    }

}
