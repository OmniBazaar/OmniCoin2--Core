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
        // Hard drive ID and MAC address of the receiver are stored to prevent multiple bonuses per machine.
        fc::string drive_id;
        fc::string mac_address;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return receiver; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::welcome_bonus_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::welcome_bonus_operation, (fee)(receiver)(drive_id)(mac_address) )
