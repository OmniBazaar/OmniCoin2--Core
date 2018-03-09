#pragma once

#include <multisig_transfer.hpp>
#include <graphene/chain/transfer_evaluator.hpp>

namespace omnibazaar {

    // Evaluator for multisig_transfer_operation.
    // It is empty because transfer evaluation is handled by 'transfer_evaluator'
    // and signatures check is handled by 'proposal_object' and transaction verification.
    class multisig_transfer_evaluator : public graphene::chain::transfer_evaluator
    {
    public:
        typedef multisig_transfer_operation operation_type;
    };
}
