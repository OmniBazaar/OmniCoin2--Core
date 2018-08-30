#pragma once

#include <verification.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for KYC verification operation.
    class verification_evaluator : public graphene::chain::evaluator<verification_evaluator>
    {
    public:
        typedef verification_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const verification_operation& op );
        graphene::chain::void_result do_apply( const verification_operation& op );
    };

}
