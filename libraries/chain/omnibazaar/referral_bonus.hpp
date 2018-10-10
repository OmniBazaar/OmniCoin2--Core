#pragma once

#include <graphene/chain/protocol/base.hpp>

namespace omnibazaar {

    // Operation for Referral Bonuses.
    struct referral_bonus_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 0;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Newly registered account.
        graphene::chain::account_id_type referred_account;
        // Bonus receiver.
        graphene::chain::account_id_type referrer_account;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return referred_account; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::referral_bonus_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::referral_bonus_operation, (fee)(referred_account)(referrer_account)(extensions) )
