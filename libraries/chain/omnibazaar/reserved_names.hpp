#pragma once

#include <graphene/chain/protocol/base.hpp>

namespace omnibazaar
{
    // Operation for updating list of reserved names.
    struct reserved_names_update_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

        graphene::chain::asset fee;
        // List of new names that will be added to database.
        std::unordered_set<std::string> names_to_add;
        // List of existing names that will be removed from database.
        std::unordered_set<std::string> names_to_delete;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        graphene::chain::account_id_type fee_payer()const { return GRAPHENE_COMMITTEE_ACCOUNT; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::reserved_names_update_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::reserved_names_update_operation, (fee)(names_to_add)(names_to_delete)(extensions) )
