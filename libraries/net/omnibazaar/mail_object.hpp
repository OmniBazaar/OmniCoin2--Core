#pragma once

#include <fc/filesystem.hpp>

namespace omnibazaar {

    //sender, recipient, subject, body, read status, creation time
    class mail_object {

    public:
        std::string uuid;
        std::string sender;
        std::string recepient;
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
