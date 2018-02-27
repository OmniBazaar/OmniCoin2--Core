#include <graphene/net/mail_sender.hpp>

static const std::string DELIVERED_STR = "delivered";
static const std::string UNDELIVERED_STR = "undelivered";
static const std::string MAILS_STR = "mails";
static const std::string TXT_EXTENSION = ".txt";

omnibazaar::mail_sender::mail_sender(const std::unordered_set<graphene::net::peer_connection_ptr>& active_peer_connections, const fc::path& node_configuration_directory)
{
	this->_active_peer_connections_ptr = &active_peer_connections;
	this->_node_condiguration_directory_ptr = &node_configuration_directory;
}

omnibazaar::mail_sender::~mail_sender()
{

}

void omnibazaar::mail_sender::store_undelivered_email(const graphene::net::mail_object& mail)
{
	fc::path undelivered_path = (*_node_condiguration_directory_ptr) / MAILS_STR;
	fc::path sender_undelivered_path = undelivered_path / mail.sender / UNDELIVERED_STR;
	fc::path new_mail_path = sender_undelivered_path / (mail.uuid + TXT_EXTENSION);
	mail.write_to_file(new_mail_path);
}

void omnibazaar::mail_sender::start_mail_sending_loop()
{
	boost::thread(&omnibazaar::mail_sender::mail_sending_thread, this);
}

void omnibazaar::mail_sender::mail_sending_thread()
{
	while (1)
	{
		fc::path mail_dir_path = (*_node_condiguration_directory_ptr) / MAILS_STR;

		std::vector<fc::path> senders_dirs = get_files_in_folder(mail_dir_path);

		std::for_each(senders_dirs.begin(), senders_dirs.end(), [&](const fc::path& sender_mail_dir_path) {

			// handle undelivered mails
			handle_undelivered_mails_for_sender(sender_mail_dir_path);
			
			// handle mails that are delivered, but not notified to sender
			handle_delivered_mails_for_sender(sender_mail_dir_path);

		});

		boost::posix_time::seconds workTime(1);
		boost::this_thread::sleep(workTime);
	}
}

void omnibazaar::mail_sender::handle_undelivered_mails_for_sender(const fc::path& sender_mail_dir_path)
{
	std::vector<graphene::net::mail_object> undelivered_mails = get_mails_from_folder(sender_mail_dir_path / UNDELIVERED_STR);

	for (const graphene::net::peer_connection_ptr& peer : *_active_peer_connections_ptr)
	{
		std::vector<graphene::net::mail_object> mails_to_send_to_peer;

		// extract all the undelivered mails for the current peer
		std::copy_if(undelivered_mails.begin(), undelivered_mails.end(), std::back_inserter(mails_to_send_to_peer), [&](graphene::net::mail_object undelivered_mail) {
			return undelivered_mail.recepient == peer->wallet_name;
		});

		if (mails_to_send_to_peer.size() > 0)
		{
			//	// prepare a bulk message
			std::stringstream bulk_message_stream;
			std::for_each(mails_to_send_to_peer.begin(), mails_to_send_to_peer.end(), [&](graphene::net::mail_object unsent_mail_object) {
				bulk_message_stream << unsent_mail_object.to_string() << "~";
			});

			graphene::net::mail_message m(bulk_message_stream.str());
			peer->send_message(graphene::net::message(m));

			// remove sent files
			std::for_each(mails_to_send_to_peer.begin(), mails_to_send_to_peer.end(), [&](graphene::net::mail_object sent_mail_object) {
				fc::path old_mail_path = sender_mail_dir_path / (sent_mail_object.uuid + TXT_EXTENSION);
				fc::remove(old_mail_path);
				fc::path mail_delivered_path = sender_mail_dir_path / DELIVERED_STR / (sent_mail_object.uuid + TXT_EXTENSION);
				sent_mail_object.write_to_file(mail_delivered_path);
			});
		}
	}
}

void omnibazaar::mail_sender::handle_delivered_mails_for_sender(const fc::path& sender_mail_dir_path)
{
	std::vector<graphene::net::mail_object> delivered_mails = get_mails_from_folder(sender_mail_dir_path / DELIVERED_STR);

	for (const graphene::net::peer_connection_ptr& peer : *_active_peer_connections_ptr)
	{
		std::vector<graphene::net::mail_object> mails_delivered_to_peer;

		if (peer->wallet_name != sender_mail_dir_path.filename())
			break;

		std::for_each(delivered_mails.begin(), delivered_mails.end(), [&](const graphene::net::mail_object& delivered_mail)
		{
			graphene::net::mail_message m(delivered_mail.to_string());
			peer->send_message(graphene::net::message(m));
			fc::path delivered_mail_path = sender_mail_dir_path / DELIVERED_STR / (delivered_mail.uuid + TXT_EXTENSION);
			fc::remove(delivered_mail_path);
		});
	}
}

std::vector<graphene::net::mail_object> omnibazaar::mail_sender::get_mails_from_folder(const fc::path& path)
{
	std::vector<fc::path> files = get_files_in_folder(path);
	std::vector<graphene::net::mail_object> result;
	result.reserve(files.size());

	std::for_each(files.begin(), files.end(), [&](const fc::path& file_path) {
		graphene::net::mail_object new_mail_object;
		new_mail_object.read_from_file(file_path);
		result.push_back(new_mail_object);
	});

	return result;
}

std::vector<fc::path> omnibazaar::mail_sender::get_files_in_folder(const fc::path& path)
{
	std::vector<fc::path> result;

	if (fc::is_directory(path))
	{
		for (fc::directory_iterator itr(path); itr != fc::directory_iterator(); ++itr)
		{
			if (!itr->filename().string().empty())
			{
				result.push_back(*itr);
			}
		}
	}
	return result;
}