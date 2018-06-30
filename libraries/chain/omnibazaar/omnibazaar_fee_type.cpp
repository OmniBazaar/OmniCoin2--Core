#include <omnibazaar_fee_type.hpp>

namespace omnibazaar
{
    bool omnibazaar_fee_type::operator >=(const omnibazaar_fee_type& other)const
    {
        try
        {
            if(other.omnibazaar_fee.valid())
            {
                FC_ASSERT(omnibazaar_fee.valid(), "OmniBazaar fee is required but is not specified.");
                FC_ASSERT(omnibazaar_fee->asset_id == other.omnibazaar_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", omnibazaar_fee->asset_id)("b", other.omnibazaar_fee->asset_id));
                FC_ASSERT(*omnibazaar_fee >= *other.omnibazaar_fee, "Insufficient OmniBazaar fee.");
            }
            if(other.escrow_fee.valid())
            {
                FC_ASSERT(escrow_fee.valid(), "Escrow fee is required but is not specified.");
                FC_ASSERT(escrow_fee->asset_id == other.escrow_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", escrow_fee->asset_id)("b", other.escrow_fee->asset_id));
                FC_ASSERT(*escrow_fee >= *other.escrow_fee, "Insufficient Escrow fee.");
            }
            if(other.referrer_buyer_fee.valid())
            {
                FC_ASSERT(referrer_buyer_fee.valid(), "Referrer fee is required but is not specified.");
                FC_ASSERT(referrer_buyer_fee->asset_id == other.referrer_buyer_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", referrer_buyer_fee->asset_id)("b", other.referrer_buyer_fee->asset_id));
                FC_ASSERT(*referrer_buyer_fee >= *other.referrer_buyer_fee, "Insufficient Buyer Referrer fee.");
            }
            if(other.referrer_seller_fee.valid())
            {
                FC_ASSERT(referrer_seller_fee.valid(), "Referrer fee is required but is not specified.");
                FC_ASSERT(referrer_seller_fee->asset_id == other.referrer_seller_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", referrer_seller_fee->asset_id)("b", other.referrer_seller_fee->asset_id));
                FC_ASSERT(*referrer_seller_fee >= *other.referrer_seller_fee, "Insufficient Seller Referrer fee.");
            }
            if(other.publisher_fee.valid())
            {
                FC_ASSERT(publisher_fee.valid(), "Publisher fee is required but is not specified.");
                FC_ASSERT(publisher_fee->asset_id == other.publisher_fee->asset_id, "Asset types don't match: ${a} - ${b}",
                          ("a", publisher_fee->asset_id)("b", other.publisher_fee->asset_id));
                FC_ASSERT(*publisher_fee >= *other.publisher_fee, "Insufficient Publisher fee.");
            }

            return true;
        }
        FC_CAPTURE_AND_RETHROW((*this)(other));
    }

    graphene::chain::share_type omnibazaar_fee_type::sum()const
    {
        return    (omnibazaar_fee.valid()      ? omnibazaar_fee->amount      : 0)
                + (escrow_fee.valid()          ? escrow_fee->amount          : 0)
                + (referrer_buyer_fee.valid()  ? referrer_buyer_fee->amount  : 0)
                + (referrer_seller_fee.valid() ? referrer_seller_fee->amount : 0)
                + (publisher_fee.valid()       ? publisher_fee->amount       : 0);
    }
}
