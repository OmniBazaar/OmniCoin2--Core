#pragma once

#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/db/generic_index.hpp>

namespace omnibazaar {

    // Class used to track open escrows.
    class escrow_object : public graphene::db::abstract_object<escrow_object>
    {
    public:
        static const uint8_t space_id = graphene::chain::protocol_ids;
        static const uint8_t type_id = graphene::chain::escrow_object_type;

        // Escrow expiration time after which funds are automatically released to seller.
        fc::time_point_sec expiration_time;
        // Buyer account.
        graphene::chain::account_id_type buyer;
        // Seller account.
        graphene::chain::account_id_type seller;
        // Escrow agent account.
        graphene::chain::account_id_type escrow;
        // Funds amount reserved in escrow (excluding escrow fee).
        graphene::chain::asset amount;
        // Reserved escrow fee.
        graphene::chain::asset escrow_fee;
    };

    // Tracks all of the escrow objects that are relevant for an individual account.
    class escrow_account_index : public graphene::chain::secondary_index
    {
    public:
        virtual void object_inserted( const graphene::chain::object& obj ) override;
        virtual void object_removed( const graphene::chain::object& obj ) override;

        void remove( const graphene::chain::account_id_type& a, const graphene::chain::escrow_id_type& e );

        std::map<graphene::chain::account_id_type, std::set<graphene::chain::escrow_id_type> > account_to_escrows;
    };

    struct by_expiration{};
    typedef boost::multi_index_container<
       escrow_object,
       graphene::chain::indexed_by<
          graphene::chain::ordered_unique< graphene::chain::tag< graphene::chain::by_id >, graphene::chain::member< graphene::chain::object, graphene::chain::object_id_type, &graphene::chain::object::id > >,
          graphene::chain::ordered_non_unique< graphene::chain::tag< by_expiration >, graphene::chain::member< escrow_object, fc::time_point_sec, &escrow_object::expiration_time > >
       >
    > escrow_multi_index_container;
    typedef graphene::chain::generic_index<escrow_object, escrow_multi_index_container> escrow_index;

}

FC_REFLECT_DERIVED(omnibazaar::escrow_object, (graphene::chain::object),
                   (expiration_time)
                   (buyer)
                   (seller)
                   (escrow)
                   (amount)
                   (escrow_fee))
