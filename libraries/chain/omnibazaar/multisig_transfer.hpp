#pragma once

#include <graphene/chain/protocol/transfer.hpp>

namespace omnibazaar {

    // Class for Multi-Sig funds transfer that builds on top of regular transfer operation and allows arbitrary number of required signatures.
    // Currently it is designed to work only within proposals workflow:
    // 1) create 'multisig_transfer_operation' and specify accounts that should sign it
    // 2) create 'proposal_create_operation' and add 'multisig_transfer_operation' to 'proposed_ops'
    // 3) broadcast a transacion with 'proposal_create_operation'
    class multisig_transfer_operation : public graphene::chain::transfer_operation
    {
    public:
        struct fee_parameters_type : public graphene::chain::transfer_operation::fee_parameters_type
        {};

        // Accounts that are required to sign this operation in order for it to execute. Must not be empty.
        fc::flat_set<graphene::chain::account_id_type> signatories;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        // base_operation interface
        void validate()const;
        void get_required_active_authorities(fc::flat_set<graphene::chain::account_id_type>& accounts)const;
    };
}

FC_REFLECT_DERIVED( omnibazaar::multisig_transfer_operation::fee_parameters_type, (graphene::chain::transfer_operation::fee_parameters_type),
                    BOOST_PP_SEQ_NIL)

FC_REFLECT_DERIVED( omnibazaar::multisig_transfer_operation, (graphene::chain::transfer_operation),
                    (signatories)(extensions))
