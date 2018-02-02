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

            virtual void read_from_file(fc::path file_path);
            virtual void write_to_file(fc::path file_path);
    };

    class publisher_component: public account_object_component 
    {
        public:
            virtual ~publisher_component(){}

            virtual void read_from_file(fc::path file_path)
            {
                std::ifstream file(file_path.string());
                file >> couchbase_ip_address;
                file >> couchbase_username;
                file >> couchbase_password;
                file.close();
            }

            virtual void write_to_file(fc::path file_path)
            {
                std::ofstream file(file_path.string());
                file << couchbase_ip_address << " ";
                file << couchbase_username << " ";
                file << couchbase_password;
                file.flush();
                file.close();
            }

        public:
            std::string couchbase_ip_address;
            std::string couchbase_username;
            std::string couchbase_password;
    };

    class referrer_component: public account_object_component 
    {
        public:
            virtual ~referrer_component(){}

            virtual void read_from_file(fc::path file_path)
            {

            }

            virtual void write_to_file(fc::path file_path)
            {

            }

        public:
            std::string bitcoin_address;
    };
}
