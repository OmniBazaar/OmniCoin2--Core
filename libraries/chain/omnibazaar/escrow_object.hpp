#pragma once

#include <graphene/db/object.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/asset_object.hpp>

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
        // Funds amount reserved in escrow.
        graphene::chain::asset amount;
    };

}

FC_REFLECT_DERIVED(omnibazaar::escrow_object, (graphene::chain::object),
                   (expiration_time)
                   (buyer)
                   (seller)
                   (escrow)
                   (amount))
