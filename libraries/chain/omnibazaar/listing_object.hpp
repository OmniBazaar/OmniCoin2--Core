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
    };


    struct by_hash;
    struct by_publisher;
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
                graphene::chain::tag< by_publisher >,
                graphene::chain::member< listing_object, graphene::chain::account_id_type, &listing_object::publisher >
            >
        >
    > listing_multi_index_container;
    typedef graphene::chain::generic_index<listing_object, listing_multi_index_container> listing_index;
}

FC_REFLECT_DERIVED(omnibazaar::listing_object, (graphene::chain::object),
                   (seller)
                   (publisher)
                   (price)
                   (listing_hash))
