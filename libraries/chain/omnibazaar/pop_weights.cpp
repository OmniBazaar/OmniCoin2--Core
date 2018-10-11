#include <pop_weights.hpp>

namespace omnibazaar
{
    uint32_t pop_weights::total()const
    {
        return referral
                + listings
                + reputation
                + trust
                + reliability
                + verification;
    }

    uint16_t pop_weights::calc_pop_score(const uint32_t referral_score,
                                         const uint32_t listings_score,
                                         const uint32_t reputation_score,
                                         const uint32_t trust_score,
                                         const uint32_t reliability_score,
                                         const bool verified
                                         )const
    {
        return (referral_score          * referral
                + listings_score        * listings
                + reputation_score      * reputation
                + trust_score           * trust
                + reliability_score     * reliability
                + (verified ? 1u : 0u)  * verification
                ) / GRAPHENE_100_PERCENT;

    }
}
