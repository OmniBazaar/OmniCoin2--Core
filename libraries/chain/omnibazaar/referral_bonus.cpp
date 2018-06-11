#include <referral_bonus.hpp>

namespace omnibazaar {

    void referral_bonus_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        FC_ASSERT( referred_account != referrer_account );
    }

}
