#include <welcome_bonus.hpp>

namespace omnibazaar {

    void welcome_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( !drive_id.empty() );
        FC_ASSERT( !mac_address.empty() );
    }

}
