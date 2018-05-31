#include <escrow_evaluator.hpp>
#include <escrow_object.hpp>
#include <listing_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    graphene::chain::void_result escrow_create_evaluator::do_evaluate( const escrow_create_operation& op )
    {
        try
        {
            escrow_ddump((op));

            const graphene::chain::database& d = db();
            const auto& global_parameters = d.get_global_properties().parameters;

            // Check mutually acceptable escrow agents.
            escrow_ddump((op.buyer(d).escrows)(op.seller(d).escrows));
            FC_ASSERT( op.buyer(d).escrows.find(op.escrow) != op.buyer(d).escrows.end(), "Escrow agent is not buyer's acceptable list of agents." );
            FC_ASSERT( op.seller(d).escrows.find(op.escrow) != op.seller(d).escrows.end(), "Escrow agent is not seller's acceptable list of agents." );

            // Check funds.
            FC_ASSERT( d.get_balance(op.buyer, op.amount.asset_id).amount >= op.amount.amount,
                       "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' to escrow from account '${a}'",
                       ("a", op.buyer(d).name)
                       ("total_transfer", d.to_pretty_string(op.amount))
                       ("balance", d.to_pretty_string(d.get_balance(op.buyer, op.amount.asset_id))) );

            // Check expiration time.
            escrow_ddump((d.head_block_time())(global_parameters.maximum_escrow_lifetime));
            FC_ASSERT( op.expiration_time > d.head_block_time(), "Escrow has already expired on creation." );
            FC_ASSERT( op.expiration_time <= (d.head_block_time() + global_parameters.maximum_escrow_lifetime),
                       "Escrow expiration time is too far in the future.");

            // Check authorities.
            fc::flat_set<graphene::chain::account_id_type> auths;
            std::vector<graphene::chain::authority> other;
            graphene::chain::operation_get_required_authorities(op, auths, auths, other);
            escrow_ddump((auths)(other));

            const auto auths_predicate = [op](const graphene::chain::authority& a) { return a.account_auths.find(op.buyer) != a.account_auths.end(); };
            FC_ASSERT( (auths.find(op.buyer) != auths.end())
                       || (std::find_if(other.cbegin(), other.cend(), auths_predicate) != other.cend()),
                       "Missing buyer authority for escrow create operation");

            if(op.listing)
            {
                const omnibazaar::listing_object listing = (*op.listing)(d);
                FC_ASSERT( listing.quantity >= (*op.listing_count), "Insufficient items in stock." );
                FC_ASSERT( op.amount.asset_id == listing.price.asset_id );
                FC_ASSERT( op.amount.amount >= (listing.price.amount * (*op.listing_count)), "Amount is insufficient to buy specified listing." );
                FC_ASSERT( op.seller == listing.seller, "Transfer destination is not listing seller." );
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::db::object_id_type escrow_create_evaluator::do_apply( const escrow_create_operation& op )
    {
        try
        {
            escrow_ddump((op));

            graphene::chain::database& d = db();

            // Create escrow object.
            escrow_dlog("Creating escrow object.");
            const escrow_object& escrow = d.create<escrow_object>([&](escrow_object& e) {
                e.expiration_time = op.expiration_time;
                e.buyer = op.buyer;
                e.seller = op.seller;
                e.escrow = op.escrow;
                e.transfer_to_escrow = op.transfer_to_escrow;
                // Calculate fee and store it separately.
                const graphene::chain::share_type fee = graphene::chain::cut_fee(op.amount.amount, op.escrow(d).escrow_fee);
                e.escrow_fee = graphene::chain::asset(fee, op.amount.asset_id);
                // Store amount excluding the fee.
                e.amount = op.amount - e.escrow_fee;
                e.listing = op.listing;
                e.listing_count = op.listing_count;
            });

            // Lock buyer funds. Deduct entire amount (including escrow fee).
            escrow_dlog("Adjusting buyer balance.");
            d.adjust_balance(op.buyer, -op.amount);

            // Store funds in escrow account.
            if(op.transfer_to_escrow)
            {
                escrow_dlog("Adjusting escrow balance.");
                d.adjust_balance(op.escrow, op.amount);
            }

            return escrow.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_release_evaluator::do_evaluate( const escrow_release_operation& op )
    {
        try
        {
            escrow_ddump((op));

            const graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);
            escrow_ddump((escrow_obj));

            // Check that this operation specifies correct accounts.
            FC_ASSERT( op.buyer_account == escrow_obj.buyer, "Buyer specified in this operation doesn't match initial buyer." );
            FC_ASSERT( op.escrow_account == escrow_obj.escrow, "Escrow agent specified in this operation doesn't match initial agent." );
            FC_ASSERT( op.seller_account == escrow_obj.seller, "Seller specified in this operation doesn't match initial seller." );

            if(escrow_obj.transfer_to_escrow)
            {
                // Check escrow funds.
                FC_ASSERT( d.get_balance(op.escrow_account, escrow_obj.amount.asset_id).amount >= escrow_obj.amount.amount,
                           "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' from escrow '${e}' to account '${a}'",
                           ("a", escrow_obj.seller(d).name)
                           ("e", op.escrow_account(d).name)
                           ("total_transfer", d.to_pretty_string(escrow_obj.amount))
                           ("balance", d.to_pretty_string(d.get_balance(op.escrow_account, escrow_obj.amount.asset_id))) );
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_release_evaluator::do_apply( const escrow_release_operation& op )
    {
        try
        {
            escrow_ddump((op));

            graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);
            escrow_ddump((escrow_obj));

            if(escrow_obj.transfer_to_escrow)
            {
                // Funds were initially stored in escrow account, so remove base amount, but keep escrow fee.
                escrow_dlog("Removing funds from escrow account.");
                d.adjust_balance(escrow_obj.escrow, -escrow_obj.amount);
            }
            else
            {
                // Funds were locked in blockchain, so just pay escrow fee.
                escrow_dlog("Sending escrow fee.");
                d.adjust_balance(escrow_obj.escrow, escrow_obj.escrow_fee);
            }

            // Send remaining funds to seller.
            escrow_dlog("Adjusting seller balance.");
            d.adjust_balance(escrow_obj.seller, escrow_obj.amount);

            // Update reputation votes.
            const std::vector<std::pair<graphene::chain::account_id_type, uint16_t>> reputations = {
                { escrow_obj.seller,    op.reputation_vote_for_seller },
                { escrow_obj.buyer,     op.reputation_vote_for_buyer },
                { escrow_obj.escrow,    op.reputation_vote_for_escrow }
            };
            for(const auto& reputation : reputations)
            {
                // Can't vote for yourself.
                if(reputation.first == op.fee_paying_account)
                    continue;

                graphene::chain::account_object::update_reputation(d, reputation.first, op.fee_paying_account, reputation.second, escrow_obj.amount);
            }

            // Update listing quantity
            if(escrow_obj.listing)
            {
                escrow_dlog("Updating listing quantity.");
                d.modify((*escrow_obj.listing)(d), [&](listing_object& listing){
                    if(listing.quantity > 0)
                    {
                        listing.quantity -= *escrow_obj.listing_count;
                    }
                });
            }

            // Remove escrow thus closing the process.
            escrow_dlog("Removing escrow object.");
            d.remove(escrow_obj);

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_return_evaluator::do_evaluate( const escrow_return_operation& op )
    {
        try
        {
            escrow_ddump((op));

            const graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);
            escrow_ddump((escrow_obj));

            // Check that this operation specifies correct accounts.
            FC_ASSERT( op.seller_account == escrow_obj.seller, "Seller specified in this operation doesn't match initial seller." );
            FC_ASSERT( op.escrow_account == escrow_obj.escrow, "Escrow agent specified in this operation doesn't match initial agent." );
            FC_ASSERT( op.buyer_account == escrow_obj.buyer, "Buyer specified in this operation doesn't match initial buyer." );

            if(escrow_obj.transfer_to_escrow)
            {
                // Check escrow funds.
                FC_ASSERT( d.get_balance(op.escrow_account, escrow_obj.amount.asset_id).amount >= escrow_obj.amount.amount,
                           "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' from escrow '${e}' to account '${a}'",
                           ("a", escrow_obj.buyer(d).name)
                           ("e", op.escrow_account(d).name)
                           ("total_transfer", d.to_pretty_string(escrow_obj.amount))
                           ("balance", d.to_pretty_string(d.get_balance(op.escrow_account, escrow_obj.amount.asset_id))) );
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result escrow_return_evaluator::do_apply( const escrow_return_operation& op )
    {
        try
        {
            escrow_ddump((op));

            graphene::chain::database& d = db();
            const escrow_object& escrow_obj = op.escrow(d);
            escrow_ddump((escrow_obj));

            if(escrow_obj.transfer_to_escrow)
            {
                // Funds were initially stored in escrow account, so remove base amount, but keep escrow fee.
                escrow_dlog("Removing funds from escrow account.");
                d.adjust_balance(escrow_obj.escrow, -escrow_obj.amount);
            }
            else
            {
                // Funds were locked in blockchain, so just pay escrow fee.
                escrow_dlog("Sending escrow fee.");
                d.adjust_balance(escrow_obj.escrow, escrow_obj.escrow_fee);
            }

            // Return remaining funds to buyer.
            escrow_dlog("Adjusting buyer balance.");
            d.adjust_balance(escrow_obj.buyer, escrow_obj.amount);

            // Update reputation votes.
            const std::vector<std::pair<graphene::chain::account_id_type, uint16_t>> reputations = {
                { escrow_obj.seller,    op.reputation_vote_for_seller },
                { escrow_obj.buyer,     op.reputation_vote_for_buyer },
                { escrow_obj.escrow,    op.reputation_vote_for_escrow }
            };
            for(const auto& reputation : reputations)
            {
                // Can't vote for yourself.
                if(reputation.first == op.fee_paying_account)
                    continue;

                graphene::chain::account_object::update_reputation(d, reputation.first, op.fee_paying_account, reputation.second, escrow_obj.amount);
            }

            // Remove escrow thus closing the process.
            escrow_dlog("Removing escrow object.");
            d.remove(escrow_obj);

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}
