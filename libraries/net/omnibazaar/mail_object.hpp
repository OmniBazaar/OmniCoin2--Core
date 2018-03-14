#pragma once

#include <fc/reflect/reflect.hpp>

namespace omnibazaar {

    // Class representing a single mail object.
    class mail_object {

    public:
        std::string uuid;
        std::string sender;
        std::string recipient;
        std::string subject;
        std::string body;
        bool read_status = false;
        uint32_t creation_time = 0;
    };
}

FC_REFLECT( omnibazaar::mail_object,
            (uuid)
            (sender)
            (recipient)
            (subject)
            (body)
            (read_status)
            (creation_time)
            )
