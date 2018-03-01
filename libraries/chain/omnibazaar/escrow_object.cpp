#include <escrow_object.hpp>

namespace omnibazaar {

    void escrow_account_index::object_inserted( const graphene::chain::object& obj )
    {
        assert( dynamic_cast<const escrow_object*>(&obj) );
        const escrow_object& e = static_cast<const escrow_object&>(obj);

        account_to_escrows[e.buyer].insert(e.id);
        account_to_escrows[e.seller].insert(e.id);
        account_to_escrows[e.escrow].insert(e.id);
    }

    void escrow_account_index::object_removed( const graphene::chain::object& obj )
    {
        assert( dynamic_cast<const escrow_object*>(&obj) );
        const escrow_object& e = static_cast<const escrow_object&>(obj);

        remove(e.buyer, e.id);
        remove(e.seller, e.id);
        remove(e.escrow, e.id);
    }

    void escrow_account_index::remove( const graphene::chain::account_id_type& a, const graphene::chain::escrow_id_type& e )
    {
        const auto it = account_to_escrows.find(a);
        if(it != account_to_escrows.end())
        {
            it->second.erase(e);
            if(it->second.empty())
            {
                account_to_escrows.erase(it->first);
            }
        }
    }

}
