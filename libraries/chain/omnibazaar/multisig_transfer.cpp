#include <multisig_transfer.hpp>

namespace omnibazaar {

    void multisig_transfer_operation::validate()const
    {
        transfer_operation::validate();
        FC_ASSERT( !signatories.empty() );
    }

    void multisig_transfer_operation::get_required_active_authorities(fc::flat_set<graphene::chain::account_id_type>& accounts)const
    {
        transfer_operation::get_required_active_authorities(accounts);
        accounts.insert(signatories.begin(), signatories.end());
    }

}

