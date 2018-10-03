#pragma once

#include <graphene/chain/protocol/custom.hpp>

namespace omnibazaar {

    // Operation for Welcome Bonuses.
    struct welcome_bonus_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 0;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Bonus receiver.
        graphene::chain::account_id_type receiver;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return OMNIBAZAAR_WELCOME_ACCOUNT; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::welcome_bonus_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::welcome_bonus_operation, (fee)(receiver) )
