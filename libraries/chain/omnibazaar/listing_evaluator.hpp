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

}
