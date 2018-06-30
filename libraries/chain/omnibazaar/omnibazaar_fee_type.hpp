#pragma once

#include <graphene/chain/protocol/asset.hpp>
#include <fc/optional.hpp>

namespace omnibazaar
{
    // Contains fees specific to OmniBazaar.
    // Exact set of fees will depend on operation used.
    struct omnibazaar_fee_type
    {
        // Fee paid to "omnibazaar" from sale transfers.
        fc::optional<graphene::chain::asset> omnibazaar_fee;
        // Fee paid to escrow agent from escrow transfers.
        fc::optional<graphene::chain::asset> escrow_fee;
        // Fee paid to buyer's referrer from sale transfers.
        fc::optional<graphene::chain::asset> referrer_buyer_fee;
        // Fee paid to seller's referrer from sale transfers.
        fc::optional<graphene::chain::asset> referrer_seller_fee;
        // Fee paid to publisher hosting listings.
        fc::optional<graphene::chain::asset> publisher_fee;

        // Calculate sum of all present fees.
        graphene::chain::share_type sum()const;

        // Check if fees in this object are not less than in reference object.
        bool operator >=(const omnibazaar_fee_type& other)const;
    };
}

FC_REFLECT( omnibazaar::omnibazaar_fee_type,
            (omnibazaar_fee)
            (escrow_fee)
            (referrer_buyer_fee)
            (referrer_seller_fee)
            (publisher_fee))
