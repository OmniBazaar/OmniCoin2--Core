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

            // Check mutually acceptable escrow agents.
            FC_ASSERT( op.buyer(d).escrows.find(op.escrow) != op.buyer(d).escrows.end(), "Escrow agent is not buyer's acceptable list of agents." );
            FC_ASSERT( op.seller(d).escrows.find(op.escrow) != op.seller(d).escrows.end(), "Escrow agent is not seller's acceptable list of agents." );

            // Check funds.
            FC_ASSERT( d.get_balance(op.buyer, op.amount.asset_id).amount >= op.amount.amount,
                       "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' to escrow from account '${a}'",
                       ("a", op.buyer(d).name)
                       ("total_transfer", d.to_pretty_string(op.amount))
                       ("balance", d.to_pretty_string(d.get_balance(op.buyer, op.amount.asset_id))) );

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
                // Calculate fee and store it separately.
                const graphene::chain::share_type fee = graphene::chain::cut_fee(op.amount.amount, op.escrow(d).escrow_fee);
                e.escrow_fee = graphene::chain::asset(fee, op.amount.asset_id);
                // Store amount excluding the fee.
                e.amount = op.amount - e.escrow_fee;
            });

            // Lock buyer funds. Deduct entire amount (including escrow fee).
            d.adjust_balance(op.buyer, -op.amount);

            return escrow.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_release_evaluator::do_evaluate( const escrow_release_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);

            // Check that this operation specifies correct accounts.
            FC_ASSERT( op.buyer_account == escrow_obj.buyer, "Buyer specified in this operation doesn't match initial buyer." );
            FC_ASSERT( op.escrow_account == escrow_obj.escrow, "Escrow agent specified in this operation doesn't match initial agent." );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_release_evaluator::do_apply( const escrow_release_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);

            // Pay escrow fee.
            d.adjust_balance(escrow_obj.escrow, escrow_obj.escrow_fee);

            // Send remaining funds to seller.
            d.adjust_balance(escrow_obj.seller, escrow_obj.amount);

            // Remove escrow thus closing the process.
            d.remove(escrow_obj);

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_return_evaluator::do_evaluate( const escrow_return_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);

            // Check that this operation specifies correct accounts.
            FC_ASSERT( op.seller_account == escrow_obj.seller, "Seller specified in this operation doesn't match initial seller." );
            FC_ASSERT( op.escrow_account == escrow_obj.escrow, "Escrow agent specified in this operation doesn't match initial agent." );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_return_evaluator::do_apply( const escrow_return_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);

            // Pay escrow fee
            d.adjust_balance(escrow_obj.escrow, escrow_obj.escrow_fee);

            // Return remaining funds to buyer.
            d.adjust_balance(escrow_obj.buyer, escrow_obj.amount);

            // Remove escrow thus closing the process.
            d.remove(escrow_obj);

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}
