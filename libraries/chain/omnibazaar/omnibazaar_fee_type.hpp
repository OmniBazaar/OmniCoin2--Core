#pragma once

#include <graphene/chain/protocol/asset.hpp>
#include <fc/optional.hpp>

namespace graphene { namespace chain { class database; } }

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
        bool is_enough(const omnibazaar_fee_type& reference_fee)const;

        // Set sale fees for this object depending on listing priority and Referral participation.
        // Implicitly clears/overwrites existing sale fee values, if any.
        void set_sale_fees(const graphene::chain::database& db,
                           const listing_object& listing,
                           const graphene::chain::asset& amount,
                           const graphene::chain::account_id_type& buyer,
                           const graphene::chain::account_id_type& seller);
    };
}

FC_REFLECT( omnibazaar::omnibazaar_fee_type,
            (omnibazaar_fee)
            (escrow_fee)
            (referrer_buyer_fee)
            (referrer_seller_fee)
            (publisher_fee))
