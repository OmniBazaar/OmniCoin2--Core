#pragma once

#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/object.hpp>

namespace omnibazaar
{
    // Class containing list of "reserved" account names.
    class reserved_names_object : public graphene::db::abstract_object<reserved_names_object>
    {
    public:
        static const uint8_t space_id = graphene::chain::implementation_ids;
        static const uint8_t type_id  = graphene::chain::impl_reserved_names_object_type;

        // Current set of words.
        std::unordered_set<std::string> names;

        // Pending updates to words list.
        std::unordered_set<std::string> pending_names_to_add;
        std::unordered_set<std::string> pending_names_to_delete;
    };
}

FC_REFLECT_DERIVED( omnibazaar::reserved_names_object, (graphene::db::object),
                    (names)
                    (pending_names_to_add)
                    (pending_names_to_delete)
                    )
