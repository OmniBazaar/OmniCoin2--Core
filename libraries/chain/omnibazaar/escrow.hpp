#pragma once

#include <graphene/chain/protocol/base.hpp>

namespace omnibazaar {

    // Operation for initialting Escrow process.
    struct escrow_create_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee            = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Escrow expiration time after which funds are automatically released to seller.
        fc::time_point_sec expiration_time;
        // Buyer account.
        graphene::chain::account_id_type buyer;
        // Seller account.
        graphene::chain::account_id_type seller;
        // Escrow agent account.
        graphene::chain::account_id_type escrow;
        // Funds amount reserved in escrow.
        graphene::chain::asset amount;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return buyer; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
        void get_required_authorities( std::vector<graphene::chain::authority>& auths)const;
        void get_required_active_authorities( fc::flat_set<graphene::chain::account_id_type>& auths)const;
    };

}

FC_REFLECT( omnibazaar::escrow_create_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( omnibazaar::escrow_create_operation,
            (fee)
            (expiration_time)
            (buyer)
            (seller)
            (escrow)
            (amount))
