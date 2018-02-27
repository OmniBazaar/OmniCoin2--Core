#pragma once

#include <vector>
#include <unordered_set>
#include <graphene/net/mail_object.hpp>
#include <graphene/net/node.hpp>
#include <graphene/net/peer_connection.hpp>
#include <fc/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/timer.hpp>

namespace omnibazaar {

	class mail_sender {

	public:
		mail_sender(const std::unordered_set<graphene::net::peer_connection_ptr>& active_peer_connections, const fc::path& node_configuration_directory);
		~mail_sender();

		void store_undelivered_email(const graphene::net::mail_object& mail);
		void start_mail_sending_loop();
	
	private:
		void mail_sending_thread();
		void handle_undelivered_mails_for_sender(const fc::path& sender_mail_dir_path);
		void handle_delivered_mails_for_sender(const fc::path& sender_mail_dir_path);
		std::vector<graphene::net::mail_object> get_mails_from_folder(const fc::path& path);
		std::vector<fc::path> get_files_in_folder(const fc::path& path);

	private:
		const std::unordered_set<graphene::net::peer_connection_ptr>* _active_peer_connections_ptr;
		const fc::path* _node_condiguration_directory_ptr;
	};
}
