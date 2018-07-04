/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/chain/protocol/transfer.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <../omnibazaar/listing_object.hpp>

namespace graphene { namespace chain {

share_type transfer_operation::calculate_fee( const fee_parameters_type& schedule )const
{
    // Purchases have zero fees.
   share_type core_fee_required = listing.valid() ? 0 : schedule.fee;
   if( memo )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte );
   return core_fee_required;
}


void transfer_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( from != to );
   FC_ASSERT( amount.amount > 0 );
   FC_ASSERT( (reputation_vote >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote <= OMNIBAZAAR_REPUTATION_MAX) );
   if(listing.valid())
   {
       FC_ASSERT( listing_count.valid(), "Listing ID is specified but count is not." );
   }
   if(listing_count.valid())
   {
       FC_ASSERT( (*listing_count) > 0 );
       FC_ASSERT( listing.valid(), "Listing count is specified but listing ID is not." );
   }
}


omnibazaar::omnibazaar_fee_type transfer_operation::calculate_omnibazaar_fee(const database& db)const
{
    omnibazaar::omnibazaar_fee_type fees;
    // Add sale fees.
    if(listing.valid())
    {
        // Add OmniBazaar fee based on listing priority.
        const omnibazaar::listing_object& listing_obj = (*listing)(db);
        const share_type omnibazaar_fee = graphene::chain::cut_fee(amount.amount, listing_obj.priority_fee);
        if(omnibazaar_fee > 0)
        {
            fees.omnibazaar_fee = asset(omnibazaar_fee, amount.asset_id);
        }

        // Add any referral fees only if seller opted in to Referral program.
        if(to(db).is_referrer)
        {
            // Add fee if Buyer's referrer opted in to Referral program.
            if(from(db).referrer(db).is_referrer)
            {
                const share_type referrer_buyer_fee = graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4);
                if(referrer_buyer_fee > 0)
                {
                    fees.referrer_buyer_fee = asset(referrer_buyer_fee, amount.asset_id);
                }
            }
            // Add fee if Seller's referrer opted in to Referral program.
            if(to(db).referrer(db).is_referrer)
            {
                const share_type referrer_seller_fee = graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4);
                if(referrer_seller_fee > 0)
                {
                    fees.referrer_seller_fee = asset(referrer_seller_fee, amount.asset_id);
                }
            }
        }
    }
    return fees;
}

share_type override_transfer_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   share_type core_fee_required = schedule.fee;
   if( memo )
      core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), schedule.price_per_kbyte );
   return core_fee_required;
}


void override_transfer_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( from != to );
   FC_ASSERT( amount.amount > 0 );
   FC_ASSERT( issuer != from );
}

} } // graphene::chain
