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

#include <graphene/chain/get_config.hpp>
#include <graphene/chain/config.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

fc::variant_object get_config()
{
   fc::mutable_variant_object result;

   result[ "GRAPHENE_SYMBOL" ] = GRAPHENE_SYMBOL;
   result[ "GRAPHENE_ADDRESS_PREFIX" ] = GRAPHENE_ADDRESS_PREFIX;
   result[ "GRAPHENE_MIN_ACCOUNT_NAME_LENGTH" ] = GRAPHENE_MIN_ACCOUNT_NAME_LENGTH;
   result[ "GRAPHENE_MAX_ACCOUNT_NAME_LENGTH" ] = GRAPHENE_MAX_ACCOUNT_NAME_LENGTH;
   result[ "GRAPHENE_MIN_ASSET_SYMBOL_LENGTH" ] = GRAPHENE_MIN_ASSET_SYMBOL_LENGTH;
   result[ "GRAPHENE_MAX_ASSET_SYMBOL_LENGTH" ] = GRAPHENE_MAX_ASSET_SYMBOL_LENGTH;
   result[ "GRAPHENE_MAX_SHARE_SUPPLY" ] = GRAPHENE_MAX_SHARE_SUPPLY;
   result[ "GRAPHENE_MAX_PAY_RATE" ] = GRAPHENE_MAX_PAY_RATE;
   result[ "GRAPHENE_MAX_SIG_CHECK_DEPTH" ] = GRAPHENE_MAX_SIG_CHECK_DEPTH;
   result[ "GRAPHENE_MIN_TRANSACTION_SIZE_LIMIT" ] = GRAPHENE_MIN_TRANSACTION_SIZE_LIMIT;
   result[ "GRAPHENE_MIN_BLOCK_INTERVAL" ] = GRAPHENE_MIN_BLOCK_INTERVAL;
   result[ "GRAPHENE_MAX_BLOCK_INTERVAL" ] = GRAPHENE_MAX_BLOCK_INTERVAL;
   result[ "GRAPHENE_DEFAULT_BLOCK_INTERVAL" ] = GRAPHENE_DEFAULT_BLOCK_INTERVAL;
   result[ "GRAPHENE_DEFAULT_MAX_TRANSACTION_SIZE" ] = GRAPHENE_DEFAULT_MAX_TRANSACTION_SIZE;
   result[ "GRAPHENE_DEFAULT_MAX_BLOCK_SIZE" ] = GRAPHENE_DEFAULT_MAX_BLOCK_SIZE;
   result[ "GRAPHENE_DEFAULT_MAX_TIME_UNTIL_EXPIRATION" ] = GRAPHENE_DEFAULT_MAX_TIME_UNTIL_EXPIRATION;
   result[ "GRAPHENE_DEFAULT_MAINTENANCE_INTERVAL" ] = GRAPHENE_DEFAULT_MAINTENANCE_INTERVAL;
   result[ "GRAPHENE_DEFAULT_MAINTENANCE_SKIP_SLOTS" ] = GRAPHENE_DEFAULT_MAINTENANCE_SKIP_SLOTS;
   result[ "GRAPHENE_MIN_UNDO_HISTORY" ] = GRAPHENE_MIN_UNDO_HISTORY;
   result[ "GRAPHENE_MAX_UNDO_HISTORY" ] = GRAPHENE_MAX_UNDO_HISTORY;
   result[ "GRAPHENE_MIN_BLOCK_SIZE_LIMIT" ] = GRAPHENE_MIN_BLOCK_SIZE_LIMIT;
   result[ "GRAPHENE_MIN_TRANSACTION_EXPIRATION_LIMIT" ] = GRAPHENE_MIN_TRANSACTION_EXPIRATION_LIMIT;
   result[ "GRAPHENE_BLOCKCHAIN_PRECISION" ] = GRAPHENE_BLOCKCHAIN_PRECISION;
   result[ "GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS" ] = GRAPHENE_BLOCKCHAIN_PRECISION_DIGITS;
   result[ "GRAPHENE_DEFAULT_TRANSFER_FEE" ] = GRAPHENE_DEFAULT_TRANSFER_FEE;
   result[ "GRAPHENE_MAX_INSTANCE_ID" ] = GRAPHENE_MAX_INSTANCE_ID;
   result[ "GRAPHENE_100_PERCENT" ] = GRAPHENE_100_PERCENT;
   result[ "GRAPHENE_1_PERCENT" ] = GRAPHENE_1_PERCENT;
   result[ "GRAPHENE_MAX_MARKET_FEE_PERCENT" ] = GRAPHENE_MAX_MARKET_FEE_PERCENT;
   result[ "GRAPHENE_DEFAULT_FORCE_SETTLEMENT_DELAY" ] = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_DELAY;
   result[ "GRAPHENE_DEFAULT_FORCE_SETTLEMENT_OFFSET" ] = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_OFFSET;
   result[ "GRAPHENE_DEFAULT_FORCE_SETTLEMENT_MAX_VOLUME" ] = GRAPHENE_DEFAULT_FORCE_SETTLEMENT_MAX_VOLUME;
   result[ "GRAPHENE_DEFAULT_PRICE_FEED_LIFETIME" ] = GRAPHENE_DEFAULT_PRICE_FEED_LIFETIME;
   result[ "GRAPHENE_MAX_FEED_PRODUCERS" ] = GRAPHENE_MAX_FEED_PRODUCERS;
   result[ "GRAPHENE_DEFAULT_MAX_AUTHORITY_MEMBERSHIP" ] = GRAPHENE_DEFAULT_MAX_AUTHORITY_MEMBERSHIP;
   result[ "GRAPHENE_DEFAULT_MAX_ASSET_WHITELIST_AUTHORITIES" ] = GRAPHENE_DEFAULT_MAX_ASSET_WHITELIST_AUTHORITIES;
   result[ "GRAPHENE_DEFAULT_MAX_ASSET_FEED_PUBLISHERS" ] = GRAPHENE_DEFAULT_MAX_ASSET_FEED_PUBLISHERS;
   result[ "GRAPHENE_COLLATERAL_RATIO_DENOM" ] = GRAPHENE_COLLATERAL_RATIO_DENOM;
   result[ "GRAPHENE_MIN_COLLATERAL_RATIO" ] = GRAPHENE_MIN_COLLATERAL_RATIO;
   result[ "GRAPHENE_MAX_COLLATERAL_RATIO" ] = GRAPHENE_MAX_COLLATERAL_RATIO;
   result[ "GRAPHENE_DEFAULT_MAINTENANCE_COLLATERAL_RATIO" ] = GRAPHENE_DEFAULT_MAINTENANCE_COLLATERAL_RATIO;
   result[ "GRAPHENE_DEFAULT_MAX_SHORT_SQUEEZE_RATIO" ] = GRAPHENE_DEFAULT_MAX_SHORT_SQUEEZE_RATIO;
   result[ "GRAPHENE_DEFAULT_MARGIN_PERIOD_SEC" ] = GRAPHENE_DEFAULT_MARGIN_PERIOD_SEC;
   result[ "GRAPHENE_DEFAULT_MAX_WITNESSES" ] = GRAPHENE_DEFAULT_MAX_WITNESSES;
   result[ "GRAPHENE_DEFAULT_MAX_COMMITTEE" ] = GRAPHENE_DEFAULT_MAX_COMMITTEE;
   result[ "GRAPHENE_DEFAULT_MAX_PROPOSAL_LIFETIME_SEC" ] = GRAPHENE_DEFAULT_MAX_PROPOSAL_LIFETIME_SEC;
   result[ "GRAPHENE_DEFAULT_COMMITTEE_PROPOSAL_REVIEW_PERIOD_SEC" ] = GRAPHENE_DEFAULT_COMMITTEE_PROPOSAL_REVIEW_PERIOD_SEC;
   result[ "GRAPHENE_DEFAULT_NETWORK_PERCENT_OF_FEE" ] = GRAPHENE_DEFAULT_NETWORK_PERCENT_OF_FEE;
   result[ "GRAPHENE_DEFAULT_MAX_BULK_DISCOUNT_PERCENT" ] = GRAPHENE_DEFAULT_MAX_BULK_DISCOUNT_PERCENT;
   result[ "GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MIN" ] = GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MIN;
   result[ "GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MAX" ] = GRAPHENE_DEFAULT_BULK_DISCOUNT_THRESHOLD_MAX;
   result[ "GRAPHENE_DEFAULT_CASHBACK_VESTING_PERIOD_SEC" ] = GRAPHENE_DEFAULT_CASHBACK_VESTING_PERIOD_SEC;
   result[ "GRAPHENE_DEFAULT_CASHBACK_VESTING_THRESHOLD" ] = GRAPHENE_DEFAULT_CASHBACK_VESTING_THRESHOLD;
   result[ "GRAPHENE_DEFAULT_BURN_PERCENT_OF_FEE" ] = GRAPHENE_DEFAULT_BURN_PERCENT_OF_FEE;
   result[ "GRAPHENE_WITNESS_PAY_PERCENT_PRECISION" ] = GRAPHENE_WITNESS_PAY_PERCENT_PRECISION;
   result[ "GRAPHENE_DEFAULT_MAX_ASSERT_OPCODE" ] = GRAPHENE_DEFAULT_MAX_ASSERT_OPCODE;
   result[ "GRAPHENE_DEFAULT_FEE_LIQUIDATION_THRESHOLD" ] = GRAPHENE_DEFAULT_FEE_LIQUIDATION_THRESHOLD;
   result[ "GRAPHENE_DEFAULT_ACCOUNTS_PER_FEE_SCALE" ] = GRAPHENE_DEFAULT_ACCOUNTS_PER_FEE_SCALE;
   result[ "GRAPHENE_DEFAULT_ACCOUNT_FEE_SCALE_BITSHIFTS" ] = GRAPHENE_DEFAULT_ACCOUNT_FEE_SCALE_BITSHIFTS;
   result[ "GRAPHENE_MAX_WORKER_NAME_LENGTH" ] = GRAPHENE_MAX_WORKER_NAME_LENGTH;
   result[ "GRAPHENE_MAX_URL_LENGTH" ] = GRAPHENE_MAX_URL_LENGTH;
   result[ "GRAPHENE_NEAR_SCHEDULE_CTR_IV" ] = GRAPHENE_NEAR_SCHEDULE_CTR_IV;
   result[ "GRAPHENE_FAR_SCHEDULE_CTR_IV" ] = GRAPHENE_FAR_SCHEDULE_CTR_IV;
   result[ "GRAPHENE_CORE_ASSET_CYCLE_RATE" ] = GRAPHENE_CORE_ASSET_CYCLE_RATE;
   result[ "GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS" ] = GRAPHENE_CORE_ASSET_CYCLE_RATE_BITS;
   result[ "GRAPHENE_DEFAULT_WITNESS_PAY_PER_BLOCK" ] = GRAPHENE_DEFAULT_WITNESS_PAY_PER_BLOCK;
   result[ "GRAPHENE_DEFAULT_WITNESS_PAY_VESTING_SECONDS" ] = GRAPHENE_DEFAULT_WITNESS_PAY_VESTING_SECONDS;
   result[ "GRAPHENE_DEFAULT_WORKER_BUDGET_PER_DAY" ] = GRAPHENE_DEFAULT_WORKER_BUDGET_PER_DAY;
   result[ "GRAPHENE_MAX_INTEREST_APR" ] = GRAPHENE_MAX_INTEREST_APR;
   result[ "GRAPHENE_COMMITTEE_ACCOUNT" ] = GRAPHENE_COMMITTEE_ACCOUNT;
   result[ "GRAPHENE_WITNESS_ACCOUNT" ] = GRAPHENE_WITNESS_ACCOUNT;
   result[ "GRAPHENE_RELAXED_COMMITTEE_ACCOUNT" ] = GRAPHENE_RELAXED_COMMITTEE_ACCOUNT;
   result[ "GRAPHENE_NULL_ACCOUNT" ] = GRAPHENE_NULL_ACCOUNT;
   result[ "GRAPHENE_TEMP_ACCOUNT" ] = GRAPHENE_TEMP_ACCOUNT;

   return result;
}

} } // graphene::chain
