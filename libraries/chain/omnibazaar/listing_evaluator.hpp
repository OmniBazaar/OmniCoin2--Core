#pragma once

#include <escrow.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Listing create operation.
    class listing_create_evaluator : public graphene::chain::evaluator<listing_create_evaluator>
    {
    public:
        typedef listing_create_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const listing_create_operation& op );
        graphene::db::object_id_type do_apply( const listing_create_operation& op );
    };

    // Evaluator for Listing update operation.
    class listing_update_evaluator : public graphene::chain::evaluator<listing_update_evaluator>
    {
    public:
        typedef listing_update_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const listing_update_operation& op );
        graphene::chain::void_result do_apply( const listing_update_operation& op );
    };

    // Evaluator for Listing delete operation.
    class listing_delete_evaluator : public graphene::chain::evaluator<listing_delete_evaluator>
    {
    public:
        typedef listing_delete_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const listing_delete_operation& op );
        graphene::chain::void_result do_apply( const listing_delete_operation& op );
    };

    // Evaluator for Listing report operation.
    class listing_report_evaluator : public graphene::chain::evaluator<listing_report_evaluator>
    {
    public:
        typedef listing_report_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const listing_report_operation& op );
        graphene::chain::void_result do_apply( const listing_report_operation& op );
    };

}
