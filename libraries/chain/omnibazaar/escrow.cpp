#include <escrow.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <listing_object.hpp>

namespace omnibazaar {

    void escrow_create_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( amount.amount > 0 );
        FC_ASSERT( buyer != seller );
        FC_ASSERT( buyer != escrow );
        FC_ASSERT( seller != escrow );
        if(listing.valid())
        {
            FC_ASSERT( listing_count.valid(), "Listing ID is specified but count is not." );
        }
        if(listing_count.valid())
        {
            FC_ASSERT( (*listing_count) > 0 );
            FC_ASSERT( listing.valid(), "Listing count is specified but listing ID is not." );
        }
        // Vested balances work only with core asset, so for now implement support only for core asset in escrow operations.
        FC_ASSERT( amount.asset_id == graphene::chain::asset_id_type(), "Escrows support only ${c} currency.", ("c", GRAPHENE_SYMBOL) );
    }

    graphene::chain::share_type escrow_create_operation::calculate_fee(const fee_parameters_type& k)const
    {
        // Purchases have zero fees.
        graphene::chain::share_type core_fee_required = listing.valid() ? 0 : k.fee;
        if(memo)
        {
            core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), k.price_per_kbyte );
        }
        return core_fee_required;
    }

    void escrow_create_operation::get_required_authorities(std::vector<graphene::chain::authority>& auths)const
    {
        // Buyer is the only required authority for this operation.
        graphene::chain::authority auth;
        auth.add_authority(buyer, 1);
        auth.weight_threshold = 1;
        auths.emplace_back(std::move(auth));
    }

    void escrow_create_operation::get_required_active_authorities(fc::flat_set<graphene::chain::account_id_type>& auths)const
    {
        auths.insert(buyer);
    }

    omnibazaar_fee_type escrow_create_operation::calculate_omnibazaar_fee(const graphene::chain::database& db)const
    {
        omnibazaar_fee_type fees;
        // Add escrow fee.
        fees.escrow_fee = graphene::chain::asset(graphene::chain::cut_fee(amount.amount, escrow(db).escrow_fee), amount.asset_id);
        // Add sale fees.
        if(listing.valid())
        {
            // Add OmniBazaar fee based on listing priority.
            const omnibazaar::listing_object& listing_obj = (*listing)(db);
            const graphene::chain::share_type omnibazaar_fee = graphene::chain::cut_fee(amount.amount, listing_obj.priority_fee);
            if(omnibazaar_fee > 0)
            {
                fees.omnibazaar_fee = graphene::chain::asset(omnibazaar_fee, amount.asset_id);
            }

            // Add any referral fees only if seller opted in to Referral program.
            if(seller(db).is_referrer)
            {
                // Add fee if Buyer's referrer opted in to Referral program.
                if(buyer(db).referrer(db).is_referrer)
                {
                    fees.referrer_buyer_fee = graphene::chain::asset(graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4), amount.asset_id);
                }
                // Add fee if Seller's referrer opted in to Referral program.
                if(seller(db).referrer(db).is_referrer)
                {
                    fees.referrer_seller_fee = graphene::chain::asset(graphene::chain::cut_fee(amount.amount, GRAPHENE_1_PERCENT / 4), amount.asset_id);
                }
            }
        }
        return fees;
    }

    void escrow_release_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( buyer_account != seller_account );
        FC_ASSERT( buyer_account != escrow_account );
        FC_ASSERT( seller_account != escrow_account );
        FC_ASSERT( (fee_paying_account == buyer_account) || (fee_paying_account == seller_account) || (fee_paying_account == escrow_account) );
        FC_ASSERT( (reputation_vote_for_buyer >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_buyer <= OMNIBAZAAR_REPUTATION_MAX) );
        FC_ASSERT( (reputation_vote_for_escrow >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_escrow <= OMNIBAZAAR_REPUTATION_MAX) );
        FC_ASSERT( (reputation_vote_for_seller >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_seller <= OMNIBAZAAR_REPUTATION_MAX) );
        if(fee_paying_account == buyer_account)
        {
            FC_ASSERT( reputation_vote_for_buyer == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
        if(fee_paying_account == escrow_account)
        {
            FC_ASSERT( reputation_vote_for_escrow == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
        if(fee_paying_account == seller_account)
        {
            FC_ASSERT( reputation_vote_for_seller == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
    }

    graphene::chain::share_type escrow_release_operation::calculate_fee(const fee_parameters_type& k)const
    {
        graphene::chain::share_type core_fee_required = k.fee;
        if(memo)
        {
            core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), k.price_per_kbyte );
        }
        return core_fee_required;
    }

    void escrow_release_operation::get_required_authorities(std::vector<graphene::chain::authority>& auths)const
    {
        // Buyer and Escrow have equal authority weights so that either of them can authorize this operation.
        graphene::chain::authority auth;
        auth.add_authority(buyer_account, 1);
        auth.add_authority(escrow_account, 1);
        auth.weight_threshold = 1;
        auths.emplace_back(std::move(auth));
    }

    void escrow_return_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( buyer_account != seller_account );
        FC_ASSERT( buyer_account != escrow_account );
        FC_ASSERT( seller_account != escrow_account );
        FC_ASSERT( (fee_paying_account == buyer_account) || (fee_paying_account == seller_account) || (fee_paying_account == escrow_account) );
        FC_ASSERT( (reputation_vote_for_seller >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_seller <= OMNIBAZAAR_REPUTATION_MAX) );
        FC_ASSERT( (reputation_vote_for_buyer >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_buyer <= OMNIBAZAAR_REPUTATION_MAX) );
        FC_ASSERT( (reputation_vote_for_escrow >= OMNIBAZAAR_REPUTATION_MIN) && (reputation_vote_for_escrow <= OMNIBAZAAR_REPUTATION_MAX) );
        if(fee_paying_account == seller_account)
        {
            FC_ASSERT( reputation_vote_for_seller == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
        if(fee_paying_account == escrow_account)
        {
            FC_ASSERT( reputation_vote_for_escrow == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
        if(fee_paying_account == buyer_account)
        {
            FC_ASSERT( reputation_vote_for_buyer == OMNIBAZAAR_REPUTATION_DEFAULT, "User can't provide reputation vote for himself." );
        }
    }

    graphene::chain::share_type escrow_return_operation::calculate_fee(const fee_parameters_type& k)const
    {
        graphene::chain::share_type core_fee_required = k.fee;
        if(memo)
        {
            core_fee_required += calculate_data_fee( fc::raw::pack_size(memo), k.price_per_kbyte );
        }
        return core_fee_required;
    }

    void escrow_return_operation::get_required_authorities(std::vector<graphene::chain::authority>& auths)const
    {
        // Seller and Escrow have equal authority weights so that either of them can authorize this operation.
        graphene::chain::authority auth;
        auth.add_authority(seller_account, 1);
        auth.add_authority(escrow_account, 1);
        auth.weight_threshold = 1;
        auths.emplace_back(std::move(auth));
    }

}
