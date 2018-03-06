#pragma once

#include <fc/filesystem.hpp>
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
        bool read_status;
        uint32_t creation_time;

        void read_from_file(const fc::path& file_path);
        void write_to_file(const fc::path& file_path) const;

        std::string to_string() const;
        //void from_string(const std::string mail_object_string);

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
