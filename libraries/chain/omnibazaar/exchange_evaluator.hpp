#pragma once

#include <exchange.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for exchange create operation.
    class exchange_create_evaluator : public graphene::chain::evaluator<exchange_create_evaluator>
    {
    public:
        typedef exchange_create_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const exchange_create_operation& op );
        graphene::db::object_id_type do_apply( const exchange_create_operation& op );
    };

    // Evaluator for exchange complete operation.
    class exchange_complete_evaluator : public graphene::chain::evaluator<exchange_complete_evaluator>
    {
    public:
        typedef exchange_complete_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const exchange_complete_operation& op );
        graphene::chain::void_result do_apply( const exchange_complete_operation& op );
    };
}
