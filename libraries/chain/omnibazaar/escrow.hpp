#pragma once

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/protocol/memo.hpp>

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
        // Flag indicating that all funds should be kept in escrow account instead of blockchain.
        bool transfer_to_escrow;

        // User provided data encrypted to the memo key of the "to" account
        fc::optional<graphene::chain::memo_data> memo;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return buyer; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
        void get_required_authorities( std::vector<graphene::chain::authority>& auths)const;
        void get_required_active_authorities(fc::flat_set<graphene::chain::account_id_type>& auths)const;
    };

    // Operation for finishing Escrow process by releasing funds to Seller.
    struct escrow_release_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee            = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Fee payer.
        graphene::chain::account_id_type fee_paying_account;
        // Escrow object that this instance operates on.
        graphene::chain::escrow_id_type escrow;
        // Accounts that are authorized to perform this operation.
        graphene::chain::account_id_type buyer_account;
        graphene::chain::account_id_type escrow_account;

        // User provided data encrypted to the memo key of the "to" account
        fc::optional<graphene::chain::memo_data> memo;

        // Rating provided by buyer or escrow for seller.
        uint16_t reputation_vote_for_seller = OMNIBAZAAR_REPUTATION_DEFAULT;
        // Rating provided by escrow for buyer.
        uint16_t reputation_vote_for_buyer = OMNIBAZAAR_REPUTATION_DEFAULT;
        // Rating provided by buyer for escrow.
        uint16_t reputation_vote_for_escrow = OMNIBAZAAR_REPUTATION_DEFAULT;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return fee_paying_account; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
        void get_required_authorities(std::vector<graphene::chain::authority>& auths)const;
    };

    // Operation for finishing Escrow process by returning funds to Buyer.
    struct escrow_return_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee            = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Fee payer.
        graphene::chain::account_id_type fee_paying_account;
        // Escrow object that this instance operates on.
        graphene::chain::escrow_id_type escrow;
        // Accounts that are authorized to perform this operation.
        graphene::chain::account_id_type seller_account;
        graphene::chain::account_id_type escrow_account;

        // User provided data encrypted to the memo key of the "to" account
        fc::optional<graphene::chain::memo_data> memo;

        // Rating provided by escrow for seller.
        uint16_t reputation_vote_for_seller = OMNIBAZAAR_REPUTATION_DEFAULT;
        // Rating provided by seller or escrow for buyer.
        uint16_t reputation_vote_for_buyer = OMNIBAZAAR_REPUTATION_DEFAULT;
        // Rating provided by seller for escrow.
        uint16_t reputation_vote_for_escrow = OMNIBAZAAR_REPUTATION_DEFAULT;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return fee_paying_account; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
        void get_required_authorities(std::vector<graphene::chain::authority>& auths)const;
    };

}

FC_REFLECT( omnibazaar::escrow_create_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( omnibazaar::escrow_create_operation,
            (fee)
            (expiration_time)
            (buyer)
            (seller)
            (escrow)
            (amount)
            (transfer_to_escrow)
            (memo))

FC_REFLECT( omnibazaar::escrow_release_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( omnibazaar::escrow_release_operation,
            (fee)
            (fee_paying_account)
            (escrow)
            (buyer_account)
            (escrow_account)
            (memo)
            (reputation_vote_for_seller)
            (reputation_vote_for_buyer)
            (reputation_vote_for_escrow))

FC_REFLECT( omnibazaar::escrow_return_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( omnibazaar::escrow_return_operation,
            (fee)
            (fee_paying_account)
            (escrow)
            (seller_account)
            (escrow_account)
            (memo)
            (reputation_vote_for_seller)
            (reputation_vote_for_buyer)
            (reputation_vote_for_escrow))
