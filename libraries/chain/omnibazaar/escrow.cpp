#include <escrow.hpp>

namespace omnibazaar {

    void escrow_create_operation::validate()const
    {
        FC_ASSERT( fee.amount > 0 );
        FC_ASSERT( amount.amount > 0 );
        FC_ASSERT( buyer != seller );
        FC_ASSERT( buyer != escrow );
        FC_ASSERT( seller != escrow );
    }

    graphene::chain::share_type escrow_create_operation::calculate_fee(const fee_parameters_type& k)const
    {
        return k.fee + calculate_data_fee( fc::raw::pack_size(*this), k.price_per_kbyte );
    }

    void escrow_create_operation::get_required_authorities(std::vector<graphene::chain::authority>& auths)const
    {
        // Buyer is the only required authority for this operation.
        graphene::chain::authority auth;
        auth.add_authority(buyer, 1);
        auth.weight_threshold = 1;
        auths.emplace_back(std::move(auth));
    }

    void escrow_create_operation::get_required_active_authorities( fc::flat_set<graphene::chain::account_id_type>& auths)const
    {
        auths.insert(buyer);
    }

}
