#pragma once

#include <iostream>
#include <fstream>
#include <graphene/chain/protocol/types.hpp>
#include <fc/filesystem.hpp>

namespace omnibazaar
{
    class account_object_component 
    {
        public:
            virtual ~account_object_component(){}

            virtual void read_from_file(const fc::path& file_path) = 0;
            virtual void write_to_file(const fc::path& file_path) const = 0;
    };

    class publisher_component: public account_object_component 
    {
        public:
            virtual ~publisher_component(){}

            virtual void read_from_file(const fc::path& file_path);
            virtual void write_to_file(const fc::path& file_path) const;

        public:
            std::string couchbase_ip_address;
            std::string couchbase_username;
            std::string couchbase_password;
    };

    class referrer_component: public account_object_component 
    {
        public:
            virtual ~referrer_component(){}

            virtual void read_from_file(const fc::path& file_path);
            virtual void write_to_file(const fc::path& file_path) const;

        public:
            std::string bitcoin_address;
    };
}
