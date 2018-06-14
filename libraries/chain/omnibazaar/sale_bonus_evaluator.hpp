#pragma once

#include <sale_bonus.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Sale bonus operation that applies operation to the database.
    class sale_bonus_evaluator : public graphene::chain::evaluator<sale_bonus_evaluator>
    {
    public:
        typedef sale_bonus_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const sale_bonus_operation& op );
        graphene::chain::asset do_apply( const sale_bonus_operation& op );

    private:
        // Calculate bonus value.
        graphene::chain::share_type get_bonus_sum()const;
    };

}

