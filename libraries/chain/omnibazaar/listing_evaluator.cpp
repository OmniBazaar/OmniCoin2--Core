#include <listing_evaluator.hpp>
#include <listing_object.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar {

    static const uint16_t DEFAULT_PUBLISHER_FEE = GRAPHENE_1_PERCENT / 4; // 0.25%

    graphene::chain::void_result listing_create_evaluator::do_evaluate( const listing_create_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();

            // Check that publisher is correct.
            const graphene::chain::account_object& publisher = op.publisher(d);
            FC_ASSERT(publisher.is_a_publisher, "Specified account is not a publisher.");

            // Check for listing duplicate.
            const auto& listings_idx = d.get_index_type<listing_index>().indices().get<by_hash>();
            FC_ASSERT(listings_idx.find(op.listing_hash) == listings_idx.cend(), "Listing already exists.");

            // Check that Seller has enough funds to pay fee to Publisher.
            const graphene::chain::share_type fee = graphene::chain::cut_fee(op.price.amount, DEFAULT_PUBLISHER_FEE);
            const graphene::chain::share_type seller_balance = d.get_balance(op.seller, op.price.asset_id).amount;
            FC_ASSERT(seller_balance >= fee, "Insufficient funds to pay fee to publisher.");

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::db::object_id_type listing_create_evaluator::do_apply( const listing_create_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Create listing object.
            const listing_object& listing = d.create<listing_object>([&](listing_object& obj) {
                obj.seller = op.seller;
                obj.publisher = op.publisher;
                obj.price = op.price;
                obj.listing_hash = op.listing_hash;
            });

            // Pay fee to publisher.
            const graphene::chain::share_type fee = graphene::chain::cut_fee(op.price.amount, DEFAULT_PUBLISHER_FEE);
            d.adjust_balance(op.publisher, graphene::chain::asset(fee, op.price.asset_id));

            return listing.id;
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_update_evaluator::do_evaluate( const listing_update_operation& op )
    {
        try
        {
            const graphene::chain::database& d = db();
            const listing_object listing = op.listing_id(d);

            // Check that seller is correct.
            FC_ASSERT(op.seller == listing.seller, "Invalid seller account.");

            // Check that publisher is correct.
            if(op.publisher.valid())
            {
                const graphene::chain::account_object& publisher = (*op.publisher)(d);
                FC_ASSERT(publisher.is_a_publisher, "Specified account is not a publisher.");
            }

            // Check for listing duplicate.
            if(op.listing_hash.valid())
            {
                const auto& listings_idx = d.get_index_type<listing_index>().indices().get<by_hash>();
                FC_ASSERT(listings_idx.find(*op.listing_hash) == listings_idx.cend(), "Listing already exists.");
            }

            // Check that Seller has enough funds to pay fee to Publisher.
            if(op.price.valid())
            {
                const graphene::chain::share_type fee = graphene::chain::cut_fee((*op.price).amount, DEFAULT_PUBLISHER_FEE);
                const graphene::chain::share_type seller_balance = d.get_balance(op.seller, (*op.price).asset_id).amount;
                FC_ASSERT(seller_balance >= fee, "Insufficient funds to pay fee to publisher.");
            }

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }

    graphene::chain::void_result listing_update_evaluator::do_apply( const listing_update_operation& op )
    {
        try
        {
            graphene::chain::database& d = db();

            // Modify listing object in blockchain.
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
            });

            // Pay fee to publisher.
            const listing_object& listing = op.listing_id(d);
            const graphene::chain::share_type fee = graphene::chain::cut_fee(listing.price.amount, DEFAULT_PUBLISHER_FEE);
            d.adjust_balance(listing.publisher, graphene::chain::asset(fee, listing.price.asset_id));

            return graphene::chain::void_result();
        }
        FC_CAPTURE_AND_RETHROW( (op) )
    }
}

