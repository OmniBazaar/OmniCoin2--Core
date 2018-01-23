#pragma once

#include <welcome_bonus.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    // Evaluator for Welcome bonus operation that applies operation to the database.
    class welcome_bonus_evaluator : public graphene::chain::evaluator<welcome_bonus_evaluator>
    {
    public:
        typedef welcome_bonus_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const welcome_bonus_operation& op );
        graphene::chain::void_result do_apply( const welcome_bonus_operation& op );

    private:
        // Calculate bonus value.
        double get_bonus_sum()const;
    };

}
