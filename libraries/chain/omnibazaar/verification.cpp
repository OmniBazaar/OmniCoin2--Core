#include <verification.hpp>

namespace omnibazaar {

    void verification_operation::validate() const
    {
        FC_ASSERT( fee.amount >= 0 );
    }
}
