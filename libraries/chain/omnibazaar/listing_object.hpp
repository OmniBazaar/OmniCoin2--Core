#pragma once

#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/db/generic_index.hpp>

namespace omnibazaar {

    // Class to track existing marketplace listings.
    class listing_object : public graphene::db::abstract_object<listing_object>
    {
    public:
        static const uint8_t space_id = graphene::chain::protocol_ids;
        static const uint8_t type_id = graphene::chain::listing_object_type;

        // User that creates and sells a product.
        graphene::chain::account_id_type seller;
        // User that hosts this product listing on his server.
        graphene::chain::account_id_type publisher;
        // Product price.
        graphene::chain::asset price;
        // Hash of listing contents.
        fc::sha256 listing_hash;
        // Quantity of product items.
        uint32_t quantity = 0;
        // Listing expiration time, after which listing is automatically removed from database.
        fc::time_point_sec expiration_time;
        // Proof Of Participation score of seller at the time of creating this listing.
        uint16_t seller_score = 0;
        // Combined PoP scores of users who reported this listing.
        uint32_t reported_score = 0;
    };

    struct by_hash;
    struct by_seller;
    struct by_publisher;
    struct by_expiration;
    typedef boost::multi_index_container<
        listing_object,
        graphene::chain::indexed_by<
            graphene::chain::ordered_unique<
                graphene::chain::tag< graphene::chain::by_id >,
                graphene::chain::member< graphene::chain::object, graphene::chain::object_id_type, &graphene::chain::object::id >
            >,
            graphene::chain::ordered_unique<
                graphene::chain::tag< by_hash >,
                graphene::chain::member< listing_object, fc::sha256, &listing_object::listing_hash >
            >,
            graphene::chain::ordered_non_unique<
                graphene::chain::tag< by_seller >,
                graphene::chain::member< listing_object, graphene::chain::account_id_type, &listing_object::seller >
            >,
            graphene::chain::ordered_non_unique<
                graphene::chain::tag< by_publisher >,
                graphene::chain::member< listing_object, graphene::chain::account_id_type, &listing_object::publisher >
            >,
            graphene::chain::ordered_non_unique<
                graphene::chain::tag< by_expiration >,
                graphene::chain::member< listing_object, fc::time_point_sec, &listing_object::expiration_time >
            >
        >
    > listing_multi_index_container;
    typedef graphene::chain::generic_index<listing_object, listing_multi_index_container> listing_index;
}

FC_REFLECT_DERIVED(omnibazaar::listing_object, (graphene::chain::object),
                   (seller)
                   (publisher)
                   (price)
                   (listing_hash)
                   (quantity)
                   (expiration_time)
                   (seller_score)
                   (reported_score))
