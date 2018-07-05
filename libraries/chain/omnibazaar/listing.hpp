#pragma once

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace omnibazaar {

    // Operation for creating marketplace listing.
    struct listing_create_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // OmniBazaar-related fees for this operation.
        omnibazaar_fee_type ob_fee;
        // User that creates and sells a product.
        graphene::chain::account_id_type seller;
        // User that hosts this product listing on his server.
        graphene::chain::account_id_type publisher;
        // Product price.
        graphene::chain::asset price;
        // Hash of listing contents.
        fc::sha256 listing_hash;
        // Quantity of product items.
        uint32_t quantity;
        // Listing priority fee.
        uint16_t priority_fee;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return seller; }
        void validate()const;
        omnibazaar_fee_type calculate_omnibazaar_fee(const graphene::chain::database &db)const;
    };

    // Operation for updating marketplace listing data.
    struct listing_update_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // OmniBazaar-related fees for this operation.
        omnibazaar_fee_type ob_fee;
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
        // Quantity of product items.
        fc::optional<uint32_t> quantity;
        // True if seller wants to extend listing registration. Involves paying publisher fee.
        bool update_expiration_time = false;
        // Listing priority fee.
        fc::optional<uint16_t> priority_fee;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return seller; }
        void validate()const;
        omnibazaar_fee_type calculate_omnibazaar_fee(const graphene::chain::database &db)const;
    };

    // Operation for removing marketplace listing from blockchain.
    struct listing_delete_operation : public graphene::chain::base_operation
    {
        // Delete has 0 basic fee to make it possible to issue this operation on listings cleanup during maintenance.
        struct fee_parameters_type {
           uint64_t fee = 0;
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
    };

    // Operation for reporting illegal listings.
    struct listing_report_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // User that reports the listing.
        graphene::chain::account_id_type reporting_account;
        // ID of listing object that will be reported as illegal.
        graphene::chain::listing_id_type listing_id;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return reporting_account; }
        void validate()const;
    };
}

FC_REFLECT(omnibazaar::listing_create_operation::fee_parameters_type, (fee))
FC_REFLECT(omnibazaar::listing_create_operation,
           (fee)
           (ob_fee)
           (seller)
           (publisher)
           (price)
           (listing_hash)
           (quantity)
           (priority_fee)
           )

FC_REFLECT(omnibazaar::listing_update_operation::fee_parameters_type, (fee))
FC_REFLECT(omnibazaar::listing_update_operation,
           (fee)
           (ob_fee)
           (seller)
           (listing_id)
           (publisher)
           (price)
           (listing_hash)
           (quantity)
           (update_expiration_time)
           (priority_fee)
           )

FC_REFLECT(omnibazaar::listing_delete_operation::fee_parameters_type, (fee))
FC_REFLECT(omnibazaar::listing_delete_operation,
           (fee)
           (seller)
           (listing_id)
           )

FC_REFLECT(omnibazaar::listing_report_operation::fee_parameters_type, (fee))
FC_REFLECT(omnibazaar::listing_report_operation,
           (fee)
           (reporting_account)
           (listing_id)
           )
