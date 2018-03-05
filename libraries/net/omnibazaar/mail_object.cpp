#include <mail_object.hpp>
#include <fstream>
#include <sstream>
#include <fc/io/json.hpp>

void omnibazaar::mail_object::read_from_file(const fc::path & file_path)
{
	std::ifstream in_file(file_path.string());
	in_file >> uuid;
	in_file >> sender;
	in_file >> recepient;
	in_file >> subject;
	in_file >> body;
	in_file >> read_status;
	in_file >> creation_time;
}

void omnibazaar::mail_object::write_to_file(const fc::path& file_path) const
{
	std::ofstream out_file(file_path.string());
	out_file << uuid;
	out_file << sender;
	out_file << recepient;
	out_file << subject;
	out_file << body;
	out_file <<  read_status;
	out_file << creation_time;
	out_file.flush();
	out_file.close();
}

std::string omnibazaar::mail_object::to_string() const
{
	std::stringstream stream;
	stream << uuid << "^";
	stream << sender << "^";
	stream << recepient << "^";
	stream << subject << "^";
	stream << body << "^";
	stream << (int)read_status << "^";
	stream << creation_time;
	return stream.str();
}

//void graphene::net::mail_object::from_string(const std::string mail_object_string)
//{
//
//}
