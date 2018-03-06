#pragma once

#include <graphene/net/node.hpp>
#include <graphene/net/peer_connection.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <mail_object.hpp>

#include <fc/filesystem.hpp>

#include <boost/thread.hpp>
#include <boost/timer.hpp>
#include <boost/asio.hpp>

#include <vector>
#include <unordered_set>


namespace omnibazaar {

	class mail_sender {

	public:
		mail_sender(const std::unordered_set<graphene::net::peer_connection_ptr>& active_peer_connections, const fc::path& node_configuration_directory);
		~mail_sender();

        void store_undelivered_email(const mail_object& mail);
		void start_mail_sending_loop();
	
	private:
		void mail_sending_tick();
		void handle_undelivered_mails_for_sender(const fc::path& sender_mail_dir_path);
		void handle_delivered_mails_for_sender(const fc::path& sender_mail_dir_path);
        std::vector<mail_object> get_mails_from_folder(const fc::path& path);

	private:
		const std::unordered_set<graphene::net::peer_connection_ptr>* _active_peer_connections_ptr;
        const fc::path* _node_configuration_directory_ptr;
		std::shared_ptr<boost::asio::deadline_timer> timer;
	};
}
