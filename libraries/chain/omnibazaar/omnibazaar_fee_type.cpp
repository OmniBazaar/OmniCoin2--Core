#include <omnibazaar_fee_type.hpp>
#include <listing_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>

namespace omnibazaar
{
    bool omnibazaar_fee_type::is_enough(const omnibazaar_fee_type& reference_fee)const
    {
        try
        {
            if(reference_fee.omnibazaar_fee.valid())
            {
                FC_ASSERT(omnibazaar_fee.valid(), "OmniBazaar fee is required but is not specified.");
                FC_ASSERT(omnibazaar_fee->asset_id == reference_fee.omnibazaar_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", omnibazaar_fee->asset_id)("b", reference_fee.omnibazaar_fee->asset_id));
                FC_ASSERT(*omnibazaar_fee >= *reference_fee.omnibazaar_fee, "Insufficient OmniBazaar fee.");
            }
            if(reference_fee.escrow_fee.valid())
            {
                FC_ASSERT(escrow_fee.valid(), "Escrow fee is required but is not specified.");
                FC_ASSERT(escrow_fee->asset_id == reference_fee.escrow_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", escrow_fee->asset_id)("b", reference_fee.escrow_fee->asset_id));
                FC_ASSERT(*escrow_fee >= *reference_fee.escrow_fee, "Insufficient Escrow fee.");
            }
            if(reference_fee.referrer_buyer_fee.valid())
            {
                FC_ASSERT(referrer_buyer_fee.valid(), "Referrer fee is required but is not specified.");
                FC_ASSERT(referrer_buyer_fee->asset_id == reference_fee.referrer_buyer_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", referrer_buyer_fee->asset_id)("b", reference_fee.referrer_buyer_fee->asset_id));
                FC_ASSERT(*referrer_buyer_fee >= *reference_fee.referrer_buyer_fee, "Insufficient Buyer Referrer fee.");
            }
            if(reference_fee.referrer_seller_fee.valid())
            {
                FC_ASSERT(referrer_seller_fee.valid(), "Referrer fee is required but is not specified.");
                FC_ASSERT(referrer_seller_fee->asset_id == reference_fee.referrer_seller_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", referrer_seller_fee->asset_id)("b", reference_fee.referrer_seller_fee->asset_id));
                FC_ASSERT(*referrer_seller_fee >= *reference_fee.referrer_seller_fee, "Insufficient Seller Referrer fee.");
            }
            if(reference_fee.publisher_fee.valid())
            {
                FC_ASSERT(publisher_fee.valid(), "Publisher fee is required but is not specified.");
                FC_ASSERT(publisher_fee->asset_id == reference_fee.publisher_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", publisher_fee->asset_id)("b", reference_fee.publisher_fee->asset_id));
                FC_ASSERT(*publisher_fee >= *reference_fee.publisher_fee, "Insufficient Publisher fee.");
            }

            return true;
        }
        FC_CAPTURE_AND_RETHROW((*this)(reference_fee));
    }

    graphene::chain::share_type omnibazaar_fee_type::sum()const
    {
        return    (omnibazaar_fee.valid()      ? omnibazaar_fee->amount      : 0)
                + (escrow_fee.valid()          ? escrow_fee->amount          : 0)
                + (referrer_buyer_fee.valid()  ? referrer_buyer_fee->amount  : 0)
                + (referrer_seller_fee.valid() ? referrer_seller_fee->amount : 0)
                + (publisher_fee.valid()       ? publisher_fee->amount       : 0);
    }

    void omnibazaar_fee_type::set_sale_fees(const graphene::chain::database& db,
                                            const listing_object &listing,
                                            const graphene::chain::asset &amount,
                                            const graphene::chain::account_id_type& buyer,
                                            const graphene::chain::account_id_type& seller)
    {
        // Clear current values.
        omnibazaar_fee = fc::optional<graphene::chain::asset>();
        referrer_buyer_fee = fc::optional<graphene::chain::asset>();
        referrer_seller_fee = fc::optional<graphene::chain::asset>();

        // Add OmniBazaar fee based on listing priority.
        const graphene::chain::share_type ob_fee = graphene::chain::cut_fee(amount.amount, listing.priority_fee);
        if(ob_fee > 0)
        {
            omnibazaar_fee = graphene::chain::asset(ob_fee, amount.asset_id);
        }

        // Add any referral fees only if seller opted in to Referral program.
        if(seller(db).is_referrer)
        {
            // Add fee if Buyer's referrer opted in to Referral program.
            if(buyer(db).referrer(db).is_referrer)
            {
                const graphene::chain::share_type ref_buyer_fee = graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4);
                if(ref_buyer_fee > 0)
                {
                    referrer_buyer_fee = graphene::chain::asset(ref_buyer_fee, amount.asset_id);
                }
            }
            // Add fee if Seller's referrer opted in to Referral program.
            if(seller(db).referrer(db).is_referrer)
            {
                const graphene::chain::share_type ref_seller_fee = graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4);
                if(ref_seller_fee > 0)
                {
                    referrer_seller_fee = graphene::chain::asset(ref_seller_fee, amount.asset_id);
                }
            }
        }
    }
}
