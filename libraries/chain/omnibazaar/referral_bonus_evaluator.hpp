#pragma once

#include <referral_bonus.hpp>
#include <graphene/chain/evaluator.hpp>

namespace omnibazaar {

    // Evaluator for Referral Bonus operation that applies operation to the database.
    class referral_bonus_evaluator : public graphene::chain::evaluator<referral_bonus_evaluator>
    {
    public:
        typedef referral_bonus_operation operation_type;

        // Methods required by graphene::chain::evaluator to process an operation.
        graphene::chain::void_result do_evaluate( const referral_bonus_operation& op );
        graphene::chain::asset do_apply( const referral_bonus_operation& op );

    private:
        // Calculate bonus value.
        graphene::chain::share_type get_bonus_sum()const;
    };

}

