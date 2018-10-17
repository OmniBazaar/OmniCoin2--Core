#include <reserved_names.hpp>
#include <omnibazaar_util.hpp>

namespace omnibazaar
{
    void reserved_names_update_operation::validate()const
    {
        FC_ASSERT( fee.amount >= 0 );
        const auto intersection = util::intersection(util::to_lower(names_to_add),
                                                     util::to_lower(names_to_delete));
        FC_ASSERT( intersection.size() <= 0,
                   "Cannot add and remove same names at the same time: ${n}.",
                   ("n", intersection) );
    }
}
