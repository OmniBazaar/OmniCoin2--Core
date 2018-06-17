#include <listing_evaluator.hpp>
#include <listing_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    static const uint16_t DEFAULT_PUBLISHER_FEE = GRAPHENE_1_PERCENT / 4; // 0.25%

    graphene::chain::void_result listing_create_evaluator::do_evaluate( const listing_create_operation& op )
    {
        try
        {
            market_ddump((op));

            const graphene::chain::database& d = db();

            // Check that publisher is correct.
            market_dlog("Checking publisher.");
            const graphene::chain::account_object& publisher = op.publisher(d);
            market_ddump((publisher));
            FC_ASSERT(publisher.is_a_publisher, "Specified account is not a publisher.");

            // Check for listing duplicate by hash.
            market_dlog("Checking listing duplicate by hash.");
            const auto& listings_idx = d.get_index_type<listing_index>().indices().get<by_hash>();
            FC_ASSERT(listings_idx.find(op.listing_hash) == listings_idx.cend(), "Listing hash already exists.");

            // Check that Seller has enough funds to pay fee to Publisher.
            market_dlog("Checking fees.");
            const graphene::chain::share_type fee = graphene::chain::cut_fee(op.price.amount, DEFAULT_PUBLISHER_FEE);
            const graphene::chain::share_type seller_balance = d.get_balance(op.seller, op.price.asset_id).amount;
            market_ddump((fee)(seller_balance));
            FC_ASSERT(seller_balance >= fee, "Insufficient funds to pay fee to publisher.");

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::db::object_id_type listing_create_evaluator::do_apply( const listing_create_operation& op )
    {
        try
        {
            market_ddump((op));

            graphene::chain::database& d = db();

            // Create listing object.
            market_dlog("Creating listing object.");
            const listing_object& listing = d.create<listing_object>([&](listing_object& obj) {
                obj.seller = op.seller;
                obj.publisher = op.publisher;
                obj.price = op.price;
                obj.listing_hash = op.listing_hash;
                obj.quantity = op.quantity;
                obj.expiration_time = d.head_block_time() + d.get_global_properties().parameters.maximum_listing_lifetime;
                obj.seller_score = op.seller(d).pop_score;
            });

            // Pay fee to publisher.
            const graphene::chain::share_type fee = graphene::chain::cut_fee(op.price.amount, DEFAULT_PUBLISHER_FEE);
            market_dlog("Paying publisher fee ${fee}", ("fee", fee));
            d.adjust_balance(op.seller, graphene::chain::asset(-fee, op.price.asset_id));
            d.adjust_balance(op.publisher, graphene::chain::asset(fee, op.price.asset_id));

            d.modify(op.publisher(d), [](graphene::chain::account_object& a){
                ++a.listings_count;
            });

            return listing.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_update_evaluator::do_evaluate( const listing_update_operation& op )
    {
        try
        {
            market_ddump((op));

            const graphene::chain::database& d = db();
            const listing_object listing = op.listing_id(d);
            market_ddump((listing));

            // Check that seller is correct.
            FC_ASSERT(op.seller == listing.seller, "Invalid seller account.");

            // Check that publisher is correct.
            if(op.publisher.valid())
            {
                const graphene::chain::account_object& publisher = (*op.publisher)(d);
                market_ddump((publisher));
                FC_ASSERT(publisher.is_a_publisher, "Specified account is not a publisher.");
            }

            // Check for listing duplicate.
            if(op.listing_hash.valid())
            {
                market_dlog("Checking listing duplicate.");
                const auto& listings_idx = d.get_index_type<listing_index>().indices().get<by_hash>();
                FC_ASSERT(listings_idx.find(*op.listing_hash) == listings_idx.cend(), "Listing already exists.");
            }

            // If Seller wants to move to another Publisher or extend listing registration time, publisher fees must be paid again.
            // Check that Seller has enough funds to pay fee to Publisher.
            if(op.update_expiration_time || op.publisher.valid())
            {
                market_dlog("Checking fees.");
                const graphene::chain::asset price = op.price.valid() ? *op.price : listing.price;
                const graphene::chain::share_type fee = graphene::chain::cut_fee((price).amount, DEFAULT_PUBLISHER_FEE);
                const graphene::chain::share_type seller_balance = d.get_balance(op.seller, (price).asset_id).amount;
                market_ddump((fee)(seller_balance));
                FC_ASSERT(seller_balance >= (fee + op.fee.amount), "Insufficient funds to pay fee to publisher.");
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_update_evaluator::do_apply( const listing_update_operation& op )
    {
        try
        {
            market_ddump((op));

            graphene::chain::database& d = db();

            // If listing is going to be moved to another publisher, update listings count for both publishers.
            if(op.publisher.valid())
            {
                const listing_object listing = op.listing_id(d);
                d.modify(listing.publisher(d), [](graphene::chain::account_object& a){
                    if(a.listings_count > 0)
                    {
                        --a.listings_count;
                    }
                });
                d.modify((*op.publisher)(d), [](graphene::chain::account_object& a){
                    ++a.listings_count;
                });
            }

            // Modify listing object in blockchain.
            market_dlog("Modifying listing object.");
            d.modify(op.listing_id(d), [&](listing_object& listing){
                if(op.listing_hash.valid())
                {
                    listing.listing_hash = *op.listing_hash;
                }
                if(op.price.valid())
                {
                    listing.price = *op.price;
                }
                if(op.publisher.valid())
                {
                    listing.publisher = *op.publisher;
                }
                if(op.quantity.valid())
                {
                    listing.quantity = *op.quantity;
                }
                if(op.update_expiration_time)
                {
                    listing.expiration_time = d.head_block_time() + d.get_global_properties().parameters.maximum_listing_lifetime;
                }
            });

            // If Seller wants to move to another Publisher or extend listing registration time, publisher fees must be paid again.
            if(op.update_expiration_time || op.publisher.valid())
            {
                const listing_object& listing = op.listing_id(d);
                market_ddump((listing));
                const graphene::chain::share_type fee = graphene::chain::cut_fee(listing.price.amount, DEFAULT_PUBLISHER_FEE);
                market_dlog("Paying publisher fee ${fee}", ("fee", fee));
                d.adjust_balance(listing.seller, graphene::chain::asset(-fee, listing.price.asset_id));
                d.adjust_balance(listing.publisher, graphene::chain::asset(fee, listing.price.asset_id));
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_delete_evaluator::do_evaluate( const listing_delete_operation& op )
    {
        try
        {
            market_ddump((op));

            const graphene::chain::database& d = db();
            const listing_object listing = op.listing_id(d);
            market_ddump((listing));

            // Check that seller is correct.
            FC_ASSERT(op.seller == listing.seller, "Invalid seller account.");

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_delete_evaluator::do_apply( const listing_delete_operation& op )
    {
        try
        {
            market_ddump((op));

            graphene::chain::database& d = db();

            const listing_object listing = op.listing_id(d);

            // Update publisher's listings count.
            d.modify(listing.publisher(d), [](graphene::chain::account_object& a){
                if(a.listings_count > 0)
                {
                    --a.listings_count;
                }
            });

            // Delete object from blockchain.
            market_dlog("Deleting listing object.");
            d.remove(op.listing_id(d));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_report_evaluator::do_evaluate( const listing_report_operation& op )
    {
        try
        {
            market_ddump((op));

            const graphene::chain::database& d = db();

            // Check that listing exists.
            const auto& listing = op.listing_id(d);
            boost::ignore_unused_variable_warning(listing);

            // Check that reporting user has any weight.
            FC_ASSERT( op.reporting_account(d).pop_score > 0, "${n} Proof of Participation score is too low.", ("n", op.reporting_account(d).name) );

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_report_evaluator::do_apply( const listing_report_operation& op )
    {
        try
        {
            market_ddump((op));

            graphene::chain::database& d = db();

            d.modify(op.listing_id(d), [&](listing_object& listing){
                listing.reported_score += op.reporting_account(d).pop_score;
            });

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}

