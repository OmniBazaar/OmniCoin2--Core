#pragma once

#include <reserved_names.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar
{
    // Evaluator for reserved_names_update_operation.
    class reserved_names_update_evaluator : public graphene::chain::evaluator<reserved_names_update_evaluator>
    {
    public:
        typedef reserved_names_update_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const reserved_names_update_operation& op );
        graphene::chain::void_result do_apply( const reserved_names_update_operation& op );
    };
}
