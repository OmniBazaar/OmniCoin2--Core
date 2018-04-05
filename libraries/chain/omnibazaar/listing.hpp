#pragma once

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace omnibazaar {

    // Operation for creating marketplace listing.
    struct listing_create_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // User that creates and sells a product.
        graphene::chain::account_id_type seller;
        // User that hosts this product listing on his server.
        graphene::chain::account_id_type publisher;
        // Product price.
        graphene::chain::asset price;
        // Hash of listing contents.
        fc::sha256 listing_hash;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return seller; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
    };
}

FC_REFLECT(omnibazaar::listing_create_operation::fee_parameters_type, (fee)(price_per_kbyte))
FC_REFLECT(omnibazaar::listing_create_operation,
           (fee)
           (seller)
           (publisher)
           (price)
           (listing_hash))
