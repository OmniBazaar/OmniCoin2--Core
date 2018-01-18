#pragma once

#include <fc/string.hpp>

namespace omnibazaar {

    // Get this computer HDD id.
    fc::string get_harddrive_id();
    // Get this computer MAC address.
    fc::string get_primary_mac();

}
