#include <mail_sender.hpp>
#include <omnibazaar_util.hpp>

omnibazaar::mail_sender::mail_sender(const std::unordered_set<graphene::net::peer_connection_ptr>& active_peer_connections)
    : _active_peer_connections_ptr(active_peer_connections)
{
}

omnibazaar::mail_sender::~mail_sender()
{
}

void omnibazaar::mail_sender::send(const omnibazaar::mail_object& mail)
{
    mail_ddump((mail));
    for (const graphene::net::peer_connection_ptr& peer : _active_peer_connections_ptr)
    {
        peer->send_message(graphene::net::mail_message(mail));
    }
}

void omnibazaar::mail_sender::send_received(const std::string mail_uuid)
{
    mail_ddump((mail_uuid));
    for (const graphene::net::peer_connection_ptr& peer : _active_peer_connections_ptr)
    {
        peer->send_message(graphene::net::mail_received_message(mail_uuid));
    }
}

void omnibazaar::mail_sender::send_confirm_received(const std::string mail_uuid)
{
    mail_ddump((mail_uuid));
    for (const graphene::net::peer_connection_ptr& peer : _active_peer_connections_ptr)
    {
        peer->send_message(graphene::net::mail_confirm_received_message(mail_uuid));
    }
}
