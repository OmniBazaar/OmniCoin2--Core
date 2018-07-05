#include <listing_evaluator.hpp>
#include <listing_object.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

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

            // Check fees.
            const omnibazaar_fee_type required_ob_fees = op.calculate_omnibazaar_fee(d);
            FC_ASSERT(op.ob_fee.is_enough(required_ob_fees), "Invalid OmniBazaar fees.");
            FC_ASSERT(!op.ob_fee.escrow_fee.valid(), "Listing does not require escrow fee.");
            FC_ASSERT(!op.ob_fee.omnibazaar_fee.valid(), "Listing does not require OmniBazaar fee.");
            FC_ASSERT(!op.ob_fee.referrer_buyer_fee.valid(), "Listing does not require buyer referrer fee.");
            FC_ASSERT(!op.ob_fee.referrer_seller_fee.valid(), "Listing does not require seller referrer fee.");
            FC_ASSERT(op.ob_fee.sum() <= op.price.amount, "Fees are larger than listing price.");

            // Check that Seller has enough funds to pay fee to Publisher.
            market_dlog("Checking fees.");
            if(op.ob_fee.publisher_fee.valid())
            {
                const graphene::chain::share_type seller_balance = d.get_balance(op.seller, op.price.asset_id).amount;
                market_ddump((*op.ob_fee.publisher_fee)(seller_balance));
                FC_ASSERT(seller_balance >= (op.ob_fee.publisher_fee->amount + op.fee.amount), "Insufficient funds to pay fee to publisher.");
            }

            FC_ASSERT(op.priority_fee <= d.get_global_properties().parameters.maximum_listing_priority_fee, "Invalid priority fee value.");

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
                obj.priority_fee = op.priority_fee;
            });

            // Pay fee to publisher.
            if(op.ob_fee.publisher_fee.valid())
            {
                market_dlog("Paying publisher fee ${fee}", ("fee", *op.ob_fee.publisher_fee));
                d.adjust_balance(op.seller, -(*op.ob_fee.publisher_fee));
                deposit_fee(op.publisher,
                            op.ob_fee.publisher_fee,
                            graphene::chain::vesting_balance_object::publisher_fee_type);
            }

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

            // Check fees.
            const omnibazaar_fee_type required_ob_fees = op.calculate_omnibazaar_fee(d);
            FC_ASSERT( op.ob_fee.is_enough(required_ob_fees), "Invalid OmniBazaar fees." );
            FC_ASSERT( !op.ob_fee.escrow_fee.valid(), "Listing does not require escrow fee." );
            FC_ASSERT( !op.ob_fee.omnibazaar_fee.valid(), "Listing does not require OmniBazaar fee." );
            FC_ASSERT( !op.ob_fee.referrer_buyer_fee.valid(), "Listing does not require buyer referrer fee." );
            FC_ASSERT( !op.ob_fee.referrer_seller_fee.valid(), "Listing does not require seller referrer fee." );
            const graphene::chain::asset final_price = op.price.valid() ? *op.price : listing.price;
            FC_ASSERT( op.ob_fee.sum() <= final_price.amount, "Fees are larger than listing price." );

            // If Seller wants to move to another Publisher or extend listing registration time, publisher fees must be paid again.
            // Check that Seller has enough funds to pay fee to Publisher.
            if(op.ob_fee.publisher_fee.valid())
            {
                market_dlog("Checking fees.");
                const graphene::chain::share_type seller_balance = d.get_balance(op.seller, op.ob_fee.publisher_fee->asset_id).amount;
                market_ddump((*op.ob_fee.publisher_fee)(seller_balance));
                FC_ASSERT(seller_balance >= (op.ob_fee.publisher_fee->amount + op.fee.amount), "Insufficient funds to pay fee to publisher.");
            }

            if(op.priority_fee.valid())
            {
                FC_ASSERT(*op.priority_fee <= d.get_global_properties().parameters.maximum_listing_priority_fee, "Invalid priority fee value.");
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
                if(op.priority_fee.valid())
                {
                    listing.priority_fee = *op.priority_fee;
                }
            });

            // If Seller wants to move to another Publisher or extend listing registration time, publisher fees must be paid again.
            if(op.ob_fee.publisher_fee.valid())
            {
                const listing_object& listing = op.listing_id(d);
                market_ddump((listing));
                market_dlog("Paying publisher fee ${fee}", ("fee", op.ob_fee));
                d.adjust_balance(listing.seller, -(*op.ob_fee.publisher_fee));
                deposit_fee(listing.publisher,
                            op.ob_fee.publisher_fee,
                            graphene::chain::vesting_balance_object::publisher_fee_type);
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

            // Check that reporting user has any weight.
            FC_ASSERT( op.reporting_account(d).pop_score > 0, "${n} Proof of Participation score is too low.", ("n", op.reporting_account(d).name) );

            // Check that reporting user did not previously report this listing.
            const listing_object& listing = op.listing_id(d);
            FC_ASSERT( listing.reported_accounts.find(op.reporting_account) == listing.reported_accounts.end(),
                       "${n} already reported listing ${l}.",
                       ("n", op.reporting_account(d).name)("l", listing.id));


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

            const graphene::chain::account_object& reporting_account = op.reporting_account(d);
            const listing_object& listing = op.listing_id(d);
            market_ddump((reporting_account)(listing));

            const uint32_t future_score = listing.reported_score + reporting_account.pop_score;
            const bool ban_listing = listing.seller_score <= 0
                    ? true
                    : (future_score / listing.seller_score) >= d.get_global_properties().parameters.listing_ban_threshold;
            market_ddump((d.get_global_properties().parameters.listing_ban_threshold)(ban_listing));

            if(ban_listing)
            {
                // Update publisher's listings count.
                d.modify(listing.publisher(d), [](graphene::chain::account_object& a){
                    if(a.listings_count > 0)
                    {
                        --a.listings_count;
                    }
                });

                // Remove listing from database, effectively banning it.
                d.remove(op.listing_id(d));
            }
            else
            {
                d.modify(op.listing_id(d), [&](listing_object& listing){
                    listing.reported_score += reporting_account.pop_score;
                    listing.reported_accounts.insert(op.reporting_account);
                });
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

}

