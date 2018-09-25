#include <founder_bonus.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/witness_object.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/smart_ref_impl.hpp>

namespace omnibazaar {

    void founder_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
    }

    bool founder_bonus_operation::check_and_add_bonus(graphene::chain::database &db,
                                                      const graphene::chain::witness_id_type witness_id,
                                                      const fc::ecc::private_key &witness_private_key)
    {
        bonus_ddump((witness_id));

        // Check that bonus is still available.
        const auto dyn_props = db.get_dynamic_global_properties();
        if(dyn_props.founder_bonus >= OMNIBAZAAR_FOUNDER_BONUS_COINS_LIMIT)
        {
            bonus_wdump(("Bonus is not available.")(dyn_props.head_block_number)(dyn_props.founder_bonus)
                        (OMNIBAZAAR_FOUNDER_BONUS_TIME_LIMIT)(OMNIBAZAAR_FOUNDER_BONUS_COINS_LIMIT));
            return false;
        }

        // Create bonus operation and transaction.
        bonus_dlog("Creating bonus operation.");
        founder_bonus_operation bonus_op;
        bonus_op.payer = witness_id(db).witness_account;

        bonus_dlog("Adding operation to transaction.");
        graphene::chain::signed_transaction tx;
        tx.operations.push_back(bonus_op);

        // Sign the transaction with witness key. Implementation is based on wallet_api::sign_transaction.
        bonus_dlog("Finalizing transaction.");
        tx.set_reference_block(dyn_props.head_block_id);
        tx.set_expiration(dyn_props.time + fc::seconds(30));
        tx.sign(witness_private_key, db.get_chain_id());
        bonus_ddump((tx));

        // Add transaction to the front of the queue so that it would definitely get into next block.
        bonus_dlog("Pushing transaction.");
        db._push_transaction(tx, false);

        return true;
    }
}
