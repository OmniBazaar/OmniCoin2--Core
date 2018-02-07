#pragma once

#include <graphene/chain/protocol/base.hpp>

// Forward declarations.
namespace graphene { namespace chain { class database; } }
namespace fc { namespace ecc { class private_key; } }

namespace omnibazaar {

    // Operation for Witness block Bonuses.
    struct witness_bonus_operation : public graphene::chain::base_operation
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

        // Check if bonus is available and add it to pending transactions during block creation.
        // Returns true if bonus was added.
        static bool check_and_add_bonus(graphene::chain::database &db, const graphene::chain::witness_id_type witness_id,
                                        const fc::ecc::private_key &witness_private_key);
    };
}

FC_REFLECT( omnibazaar::witness_bonus_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::witness_bonus_operation, (fee)(payer)(receiver) )
