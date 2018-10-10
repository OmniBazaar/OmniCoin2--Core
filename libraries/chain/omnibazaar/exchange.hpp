#pragma once

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>

namespace omnibazaar {

    // Operation for creating exchange objects.
    struct exchange_create_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee             = GRAPHENE_BLOCKCHAIN_PRECISION;
           uint32_t price_per_kbyte = 10;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Name of the currency which is represented by exchange object, e.g. "BTC" or "ETH".
        fc::string coin_name;
        // Transaction ID for specified currency.
        fc::string tx_id;
        // Account that created the transaction.
        graphene::chain::account_id_type sender;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return sender; }
        void validate()const;
        graphene::chain::share_type calculate_fee(const fee_parameters_type& k)const;
    };

    // Operation for deleting exchange objects for which exchange process is completed.
    struct exchange_complete_operation : public graphene::chain::base_operation
    {
        struct fee_parameters_type {
           uint64_t fee = 0;
        };

        // Operation fee.
        graphene::chain::asset fee;
        // Exchange object that is completed and will be removed.
        graphene::chain::exchange_id_type exchange;

        // Future extensions.
        graphene::chain::extensions_type extensions;

        // base_operation interface
        graphene::chain::account_id_type fee_payer()const { return OMNIBAZAAR_EXCHANGE_ACCOUNT; }
        void validate()const;
    };
}

FC_REFLECT( omnibazaar::exchange_create_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT( omnibazaar::exchange_create_operation,
            (fee)
            (coin_name)
            (tx_id)
            (sender)
            (extensions)
            )

FC_REFLECT( omnibazaar::exchange_complete_operation::fee_parameters_type, (fee) )
FC_REFLECT( omnibazaar::exchange_complete_operation,
            (fee)
            (exchange)
            (extensions)
            )
