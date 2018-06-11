#pragma once

#include <witness_bonus.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Witness block Bonus operation that applies operation to the database.
    class witness_bonus_evaluator : public graphene::chain::evaluator<witness_bonus_evaluator>
    {
    public:
        typedef witness_bonus_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const witness_bonus_operation& op );
        graphene::chain::void_result do_apply( const witness_bonus_operation& op );

    private:
        // Calculate bonus value.
        graphene::chain::share_type get_bonus_sum()const;
    };

}


