#pragma once

#include <fc/string.hpp>

namespace omnibazaar {

    // Class for various OmniBazaar-related utility functions.
    class util
    {
    public:
        // Get this computer HDD id.
        static fc::string get_harddrive_id();
        // Get this computer MAC address.
        static fc::string get_primary_mac();
    };
}
