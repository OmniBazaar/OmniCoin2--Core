#include <escrow_evaluator.hpp>
#include <escrow_object.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result escrow_create_evaluator::do_evaluate( const escrow_create_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();
            const auto& global_parameters = d.get_global_properties().parameters;

            // Check expiration time.
            FC_ASSERT( op.expiration_time > d.head_block_time(), "Escrow has already expired on creation." );
            FC_ASSERT( op.expiration_time <= (d.head_block_time() + global_parameters.maximum_escrow_lifetime),
                       "Escrow expiration time is too far in the future.");

            // Check authorities.
            fc::flat_set<graphene::chain::account_id_type> auths;
            std::vector<graphene::chain::authority> other;
            graphene::chain::operation_get_required_authorities(op, auths, auths, other);

            const auto auths_predicate = [op](const graphene::chain::authority& a) { return a.account_auths.find(op.buyer) != a.account_auths.end(); };
            FC_ASSERT( (auths.find(op.buyer) != auths.end())
                       || (std::find_if(other.cbegin(), other.cend(), auths_predicate) != other.cend()),
                       "Missing buyer authority for escrow create operation");

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::db::object_id_type escrow_create_evaluator::do_apply( const escrow_create_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Create escrow object.
            const escrow_object& escrow = d.create<escrow_object>([&](escrow_object& e) {
                e.expiration_time = op.expiration_time;
                e.buyer = op.buyer;
                e.seller = op.seller;
                e.escrow = op.escrow;
                e.amount = op.amount;
            });

            // Lock buyer funds.
            d.adjust_balance(op.buyer, -op.amount);

            return escrow.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}
