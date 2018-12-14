#include <exchange_evaluator.hpp>
#include <exchange_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result exchange_create_evaluator::do_evaluate( const exchange_create_operation& op )
    {
        try
        {
            exchange_ddump((op));

            const graphene::chain::database& d = db();

            // Check that specified transaction ID is not registered yet.
            const auto& exchange_idx = d.get_index_type<exchange_index>().indices().get<by_tx_id>();
            FC_ASSERT( exchange_idx.find(op.tx_id) == exchange_idx.end(), "Transaction ${tx} is already registered.",
                       ("tx", op.tx_id));

            // Only accounts that passed KYC verification can perform exchange operations.
            const graphene::chain::account_object& account = op.sender(d);
            FC_ASSERT( account.verified, "Account ${a} is not verified and cannot perform exchange operations.",
                       ("a", account.name) );

            // Check that exchange has enough funds.
            FC_ASSERT( d.get_balance(OMNIBAZAAR_EXCHANGE_ACCOUNT(d), op.amount.asset_id(d)).amount >= op.amount.amount,
                       "Exchange does not have enough balance to complete operation." );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::db::object_id_type exchange_create_evaluator::do_apply( const exchange_create_operation& op )
    {
        try
        {
            exchange_ddump((op));

            graphene::chain::database& d = db();

            exchange_dlog("Creating exchange object.");
            const exchange_object& exchange_obj = d.create<exchange_object>([&](exchange_object& e) {
                // Store coin name in case-independent format.
                e.coin_name = fc::to_lower(op.coin_name);
                e.sender = op.sender;
                e.tx_id = op.tx_id;
                e.amount = op.amount;
            });

            return exchange_obj.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result exchange_complete_evaluator::do_evaluate( const exchange_complete_operation& op )
    {
        try
        {
            exchange_ddump((op));

            const graphene::chain::database& d = db();

            // Check that exchange object exists.
            const exchange_object& obj = op.exchange(d);

            // Check that account that will receive funds from exchange is the same account
            // that originally sent funds to exchange.
            FC_ASSERT( obj.sender == op.receiver, "Receiver '${r}' was not initial sender '${s}'.",
                       ("r", op.receiver(d).name)("s", obj.sender(d).name) );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result exchange_complete_evaluator::do_apply( const exchange_complete_operation& op )
    {
        try
        {
            exchange_ddump((op));

            graphene::chain::database& d = db();

            exchange_dlog("Sending funds to '${r}'.", ("r", op.receiver(d)));
            const auto amount = op.exchange(d).amount;
            d.adjust_balance(OMNIBAZAAR_EXCHANGE_ACCOUNT, -amount);
            d.adjust_balance(op.receiver, amount);

            exchange_dlog("Removing exchange object ${obj}.", ("obj", op.exchange));
            d.remove(op.exchange(d));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }
}
