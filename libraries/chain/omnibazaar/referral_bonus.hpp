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
        // Operation fee payer.
        graphene::chain::account_id_type payer;
        // Bonus receiver.
        graphene::chain::account_id_type receiver;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return payer; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::referral_bonus_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::referral_bonus_operation, (fee)(payer)(receiver) )
