#include <witness_bonus.hpp>
#include <graphene/chain/protocol/transaction.hpp>
#include <graphene/chain/database.hpp>
#include <fc/crypto/elliptic.hpp>
#include <graphene/chain/witness_object.hpp>

namespace omnibazaar {

    void witness_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
    }

    bool witness_bonus_operation::check_and_add_bonus(graphene::chain::database &db,
                                                      const graphene::chain::witness_id_type witness_id,
                                                      const fc::ecc::private_key &witness_private_key)
    {
        const auto dyn_props = db.get_dynamic_global_properties();
        if(dyn_props.head_block_number >= OMNIBAZAAR_WITNESS_BLOCK_LIMIT)
            return false;

        // Create bonus operation and transaction.
        witness_bonus_operation bonus_op;
        bonus_op.receiver = witness_id(db).witness_account;
        bonus_op.payer = bonus_op.receiver;

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

