#pragma once

#include <graphene/net/node.hpp>
#include <graphene/net/peer_connection.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <mail_object.hpp>

#include <fc/filesystem.hpp>

#include <vector>
#include <unordered_set>

namespace omnibazaar {

    // Class for managing mail p2p communication.
    // No thread sync is required provided it is called only by graphene::net::node.
	class mail_sender {

	public:
        mail_sender(const std::unordered_set<graphene::net::peer_connection_ptr>& active_peer_connections);
		~mail_sender();

        // Send specified mail object to other nodes.
        void send(const mail_object& mail);

        // Send notification that specified mail was received.
        void send_received(const std::string mail_uuid);

        // Send notification that mail reception was confirmed by sending user.
        void send_confirm_received(const std::string mail_uuid);
	
	private:
        const std::unordered_set<graphene::net::peer_connection_ptr>& _active_peer_connections_ptr;
	};
}
