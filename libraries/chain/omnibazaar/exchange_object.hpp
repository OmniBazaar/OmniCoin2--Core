#pragma once

#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/protocol/base.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace omnibazaar {

    // Class for storing info about pending exchange operations.
    class exchange_object : public graphene::db::abstract_object<exchange_object>
    {
    public:
        static const uint8_t space_id = graphene::chain::protocol_ids;
        static const uint8_t type_id = graphene::chain::exchange_object_type;

        // Name of the currency which is represented by this object, e.g. "BTC" or "ETH".
        fc::string coin_name;
        // Transaction ID for specified currency.
        fc::string tx_id;
        // Account that created the transaction.
        graphene::chain::account_id_type sender;

        // Future extensions.
        graphene::chain::extensions_type extensions;
    };

    struct by_coin_and_id;
    struct by_tx_id;
    typedef boost::multi_index_container<
        exchange_object,
        graphene::chain::indexed_by<
            graphene::chain::ordered_unique<
                graphene::chain::tag< graphene::chain::by_id >,
                graphene::chain::member< graphene::chain::object, graphene::chain::object_id_type, &graphene::chain::object::id >
            >,
            graphene::chain::ordered_unique<
                graphene::chain::tag< by_tx_id >,
                graphene::chain::member< exchange_object, fc::string, &exchange_object::tx_id >
            >,
            graphene::chain::ordered_non_unique<
                graphene::chain::tag< by_coin_and_id >,
                boost::multi_index::composite_key<
                    exchange_object,
                    graphene::chain::member< exchange_object, fc::string, &exchange_object::coin_name >,
                    graphene::chain::member< graphene::chain::object, graphene::chain::object_id_type, &graphene::chain::object::id >
                >
            >
        >
    > exchange_multi_index_container;
    typedef graphene::chain::generic_index<exchange_object, exchange_multi_index_container> exchange_index;
}

FC_REFLECT_DERIVED(omnibazaar::exchange_object, (graphene::chain::object),
                   (coin_name)
                   (tx_id)
                   (sender)
                   (extensions)
                   )
