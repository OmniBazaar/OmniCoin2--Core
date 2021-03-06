#pragma once

#include <escrow.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Escrow create operation.
    class escrow_create_evaluator : public graphene::chain::evaluator<escrow_create_evaluator>
    {
    public:
        typedef escrow_create_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const escrow_create_operation& op );
        graphene::db::object_id_type do_apply( const escrow_create_operation& op );
    };

    // Evaluator for Escrow release operation.
    class escrow_release_evaluator : public graphene::chain::evaluator<escrow_release_evaluator>
    {
    public:
        typedef escrow_release_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const escrow_release_operation& op );
        graphene::chain::void_result do_apply( const escrow_release_operation& op );
    };

    // Evaluator for Escrow return operation.
    class escrow_return_evaluator : public graphene::chain::evaluator<escrow_return_evaluator>
    {
    public:
        typedef escrow_return_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const escrow_return_operation& op );
        graphene::chain::void_result do_apply( const escrow_return_operation& op );
    };

    // Evaluator for Escrow extend operation.
    class escrow_extend_evaluator : public graphene::chain::evaluator<escrow_extend_evaluator>
    {
    public:
        typedef escrow_extend_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const escrow_extend_operation& op );
        graphene::chain::void_result do_apply( const escrow_extend_operation& op );
    };

}
