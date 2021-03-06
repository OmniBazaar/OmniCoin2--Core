#include <verification_evaluator.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result verification_evaluator::do_evaluate( const verification_operation& op )
    {
        try
        {
            idump((op));

            const graphene::chain::database& d = db();

            const graphene::chain::account_object& account = op.account(d);
            FC_ASSERT( op.status != account.verified, "Account ${a} already has verification status '${s}'.",
                       ("a", account.name)("s", account.verified) );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result verification_evaluator::do_apply( const verification_operation& op )
    {
        try
        {
            idump((op));

            graphene::chain::database& d = db();

            d.modify(op.account(d), [&](graphene::chain::account_object& a){
                a.verified = op.status;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }
}
