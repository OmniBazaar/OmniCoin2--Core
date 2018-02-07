#include <founder_bonus.hpp>
#include <graphene/chain/database.hpp>
#include <fc/crypto/elliptic.hpp>

namespace omnibazaar {

    void founder_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
    }

    bool founder_bonus_operation::check_and_add_bonus(graphene::chain::database &db, const fc::ecc::private_key &witness_private_key)
    {
        // Check that bonus is still available.
        const auto dyn_props = db.get_dynamic_global_properties();
        if(dyn_props.head_block_number >= OMNIBAZAAR_FOUNDER_BLOCK_LIMIT)
            return false;

        // Create bonus operation and transaction.
        founder_bonus_operation bonus_op;
        bonus_op.payer = OMNIBAZAAR_FOUNDER_ACCOUNT;

        graphene::chain::signed_transaction tx;
        tx.operations.push_back(bonus_op);

        // Sign the transaction with witness key. Implementation is based on wallet_api::sign_transaction.
        tx.set_reference_block(dyn_props.head_block_id);
        tx.set_expiration(dyn_props.time + fc::seconds(30));
        tx.sign(witness_private_key, db.get_chain_id());

        // Add transaction to the front of the queue so that it would definitely get into next block.
        db._push_transaction(tx, false);

        return true;
    }
}
