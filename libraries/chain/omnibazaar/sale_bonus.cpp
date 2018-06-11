#include <sale_bonus.hpp>

namespace omnibazaar {

    void sale_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( seller != buyer );
    }

}
