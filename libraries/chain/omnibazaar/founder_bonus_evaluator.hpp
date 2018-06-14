#pragma once

#include <founder_bonus.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Founder Bonus operation that applies operation to the database.
    class founder_bonus_evaluator : public graphene::chain::evaluator<founder_bonus_evaluator>
    {
    public:
        typedef founder_bonus_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const founder_bonus_operation& op );
        graphene::chain::asset do_apply( const founder_bonus_operation& op );
    };

}


