#pragma once

#include <graphene/chain/protocol/base.hpp>

namespace omnibazaar
{
    // Class representing weights of all Proof of Participation components in final score.
    class pop_weights
    {
    public:
        uint16_t referral = 20 * GRAPHENE_1_PERCENT;
        uint16_t listings = 20 * GRAPHENE_1_PERCENT;
        uint16_t reputation = 20 * GRAPHENE_1_PERCENT;
        uint16_t trust = 20 * GRAPHENE_1_PERCENT;
        uint16_t reliability = 20 * GRAPHENE_1_PERCENT;
        uint16_t verification = 0;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        // Sum of all weights. Must always equal to 100%.
        uint32_t total()const;

        // Calculate total Proof of Participation score based on components scores and current weights.
        uint16_t calc_pop_score(const uint32_t referral_score,
                                const uint32_t listings_score,
                                const uint32_t reputation_score,
                                const uint32_t trust_score,
                                const uint32_t reliability_score,
                                const bool verified
                                )const;
    };
}

FC_REFLECT(omnibazaar::pop_weights,
           (referral)
           (listings)
           (reputation)
           (trust)
           (reliability)
           (verification)
           (extensions)
           )
