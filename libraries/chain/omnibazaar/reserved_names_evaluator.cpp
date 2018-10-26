#include <reserved_names_evaluator.hpp>
#include <reserved_names_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar
{
    graphene::chain::void_result reserved_names_update_evaluator::do_evaluate( const reserved_names_update_operation& op )
    {
        try
        {
            // This can only be executed as a committee proposal.
            FC_ASSERT( trx_state->_is_proposed_trx );

            const graphene::chain::database& d = db();

            const reserved_names_object& reserved_names = d.get_reserved_names();
            FC_ASSERT( op.names_to_delete.size() <= reserved_names.names.size(),
                       "Cannot delete more names than currently present.");
            const auto diff = util::set_difference(util::to_lower(op.names_to_delete), reserved_names.names);
            FC_ASSERT( diff.size() <= 0,
                       "Cannot delete names that are not currently present: ${d}.",
                       ("d", diff));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result reserved_names_update_evaluator::do_apply( const reserved_names_update_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Update pending add/delete lists. Actual list of names will be updated during maintenance interval.
            d.modify(d.get_reserved_names(), [&](reserved_names_object& obj){
                obj.pending_names_to_add = op.names_to_add;
                obj.pending_names_to_delete = op.names_to_delete;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }
}
