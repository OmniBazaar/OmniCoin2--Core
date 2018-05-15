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
        // Globally unique ID.
        std::string uuid;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return seller; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
    };

    // Operation for updating marketplace listing data.
    struct listing_update_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // User that creates and sells a product.
        graphene::chain::account_id_type seller;
        // ID of listing object that will be updated.
        graphene::chain::listing_id_type listing_id;
        // User that hosts this product listing on his server.
        fc::optional<graphene::chain::account_id_type> publisher;
        // Product price.
        fc::optional<graphene::chain::asset> price;
        // Hash of listing contents.
        fc::optional<fc::sha256> listing_hash;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return seller; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
    };

    // Operation for removing marketplace listing from blockchain.
    struct listing_delete_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // User that creates and sells a product.
        graphene::chain::account_id_type seller;
        // ID of listing object that will be deleted.
        graphene::chain::listing_id_type listing_id;

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
           (listing_hash)
           (uuid))

FC_REFLECT(omnibazaar::listing_update_operation::fee_parameters_type, (fee)(price_per_kbyte))
FC_REFLECT(omnibazaar::listing_update_operation,
           (fee)
           (seller)
           (listing_id)
           (publisher)
           (price)
           (listing_hash))

FC_REFLECT(omnibazaar::listing_delete_operation::fee_parameters_type, (fee)(price_per_kbyte))
FC_REFLECT(omnibazaar::listing_delete_operation,
           (fee)
           (seller)
           (listing_id))
