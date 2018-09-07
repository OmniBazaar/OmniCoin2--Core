#pragma once

#include <graphene/chain/protocol/base.hpp>

namespace omnibazaar {

    // Operation for setting "verified" flag for accounts.
    struct verification_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 0;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Account for which this operation sets "verified" flag.
        graphene::chain::account_id_type account;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return OMNIBAZAAR_KYC_ACCOUNT; }
        void validate()const;
    };

}

FC_REFLECT(omnibazaar::verification_operation::fee_parameters_type, (fee))
FC_REFLECT(omnibazaar::verification_operation,
           (fee)
           (account)
           )
