#include <../omnibazaar/account_object_components.hpp>

void omnibazaar::publisher_component::read_from_file(const fc::path& file_path)
{
    std::ifstream file(file_path.string());
    file >> couchbase_ip_address;
    file >> couchbase_username;
    file >> couchbase_password;
    file.close();
}

void omnibazaar::publisher_component::write_to_file(const fc::path& file_path) const
{
    std::ofstream file(file_path.string());
    file << couchbase_ip_address << " ";
    file << couchbase_username << " ";
    file << couchbase_password;
    file.flush();
    file.close();
}