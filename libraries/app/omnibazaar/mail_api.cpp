#include <mail_api.h>
#include <mail_object.hpp>
#include <mail_storage.hpp>
#include <graphene/app/application.hpp>

static const int TIMER_TICK_INTERVAL_IN_SECONDS = 1;

namespace omnibazaar {

    mail_api::mail_api(graphene::app::application &a)
        : _app(a)
        , _thread("mail_api")
    {
        ilog("Connecting Mail API signals.");
        _new_mail_connection = _app.p2p_node()->mail_new.connect([this](const mail_object& m){ on_new_mail(m); });
        _received_mail_connection = _app.p2p_node()->mail_received.connect([this](const std::string& u){ on_mail_received(u); });
        _confirm_received_mail_connection = _app.p2p_node()->mail_confirm_received.connect([this](const std::string& u){ on_mail_confirm_received(u); });

        ilog("Starting mail processing loop.");
        start_mail_processing_loop();
    }

    mail_api::~mail_api()
    {
        ilog("Terminating Mail API background thread.");
        _thread.quit();
    }

    void mail_api::send(callback_type cb, const mail_object& mail)
    {
        FC_ASSERT( !mail.uuid.empty(), "Mail UUID is empty" );

        // Register callback for notifying sender about delivery.
        _send_callbacks[mail.uuid] = cb;
        // Save mail to disk.
        _app.mail_storage()->store(mail);
        // Send mail to other backend nodes.
        _app.p2p_node()->mail_send(mail);
        // If receiving user is connected to this node, send mail directly.
        const auto itr = _receive_callbacks.find(mail.recipient);
        if(itr != _receive_callbacks.end())
        {
            exec_callback(itr->second, { fc::variant(mail) });
        }
    }

    void mail_api::subscribe(callback_type cb, const std::string& receiver_name)
    {
        FC_ASSERT( !receiver_name.empty(), "Mail receiver name cannot be empty." );

        // Register callback.
        _receive_callbacks[receiver_name] = cb;
        // Send any pending mails to this callback.
        const std::vector<mail_object> mails = _app.mail_storage()->get_mails_by_receiver(receiver_name);
        std::vector<fc::variant> data;
        data.reserve(mails.size());
        std::transform(mails.begin(), mails.end(), std::back_inserter(data), [](const mail_object& m){ return fc::variant(m); });
        exec_callback(cb, data);
    }

    void mail_api::set_received(const std::string& mail_uuid)
    {
        FC_ASSERT( !mail_uuid.empty(), "Mail UUID is empty." );

        // Move mail to delivered folder.
        _app.mail_storage()->set_received(mail_uuid);
        // Send notification that mail was received.
        _app.p2p_node()->mail_send_received(mail_uuid);
        // If sender is connected to this node, notify about successful mail delivery.
        const auto itr = _send_callbacks.find(mail_uuid);
        if(itr != _send_callbacks.end())
        {
            exec_callback(itr->second, { fc::variant(send_confirmation{mail_uuid}) });
        }
    }

    void mail_api::on_new_mail(const mail_object& mail)
    {
        if(mail.uuid.empty())
        {
            wlog("Invalid mail object ${mail}.", ("mail", mail));
            return;
        }

        // Find receiver.
        const auto iter = _receive_callbacks.find(mail.recipient);
        if(iter == _receive_callbacks.end())
        {
            wlog("Receiver ${rec} has no registered callback.", ("rec", mail.recipient));
            return;
        }

        exec_callback(iter->second, { fc::variant(mail) });
    }

    void mail_api::on_mail_received(const std::string& mail_uuid)
    {
        if(mail_uuid.empty())
        {
            wlog("Mail UUID is empty.");
            return;
        }

        if(_send_callbacks.empty())
        {
            wlog("Callback list is empty.");
            return;
        }

        // Find the callback.
        const auto iter = _send_callbacks.find(mail_uuid);
        if(iter == _send_callbacks.end())
        {
            wlog("No callback registered for mail ${uuid}.", ("uuid", mail_uuid));
            return;
        }

        exec_callback(iter->second, { fc::variant(send_confirmation{mail_uuid}) });
    }

    void mail_api::exec_callback(callback_type callback, const std::vector<fc::variant> &objects)
    {
        // Need to ensure the object is not deleted for the life of the async operation.
        auto capture_this = shared_from_this();

        // Invoke the callback.
        for(auto& object : objects)
        {
            fc::async( [capture_this, this, object, callback](){
               callback( object );
            } );
        }
    }

    void mail_api::start_mail_processing_loop()
    {
        if(!_thread.is_running())
            return;

        // Schedule operation in background thread.
        auto future = _thread.schedule([this](){ mail_sending_tick(); },
            fc::time_point::now() + fc::seconds(TIMER_TICK_INTERVAL_IN_SECONDS),
            "Executing mail processing tick");

        // Restart operation upon its completion.
        future.on_complete([this](const fc::exception_ptr& exptr) { start_mail_processing_loop(); });
    }

    void mail_api::mail_sending_tick()
    {
        ilog("Processing pending mails.");
        // Re-send mails that were not successfully delivered to receivers.
        const std::vector<fc::path> pending_mails = _app.mail_storage()->get_pending_mails();
        for(const auto& path : pending_mails)
        {
            if(!fc::exists(path))
                continue;

            mail_object mail;
            mail.read_from_file(path);
            // Send mail to other backend nodes.
            _app.p2p_node()->mail_send(mail);
            // If receiving user is connected to this node, send mail directly.
            const auto itr = _receive_callbacks.find(mail.recipient);
            if(itr != _receive_callbacks.end())
            {
                exec_callback(itr->second, { fc::variant(mail) });
            }
        }

        ilog("Processing delivered mails.");
        // Re-send notifications to senders for mails that were delivered to receivers.
        const std::vector<std::string> pending_notifications = _app.mail_storage()->get_received_mails();
        for(const auto& mail_uuid : pending_notifications)
        {
            // Send notification that mail was received.
            _app.p2p_node()->mail_send_received(mail_uuid);
            // If sender is connected to this node, notify about successful mail delivery.
            const auto itr = _send_callbacks.find(mail_uuid);
            if(itr != _send_callbacks.end())
            {
                exec_callback(itr->second, { fc::variant(send_confirmation{mail_uuid}) });
            }
        }
    }

    void mail_api::confirm_received(const std::string& mail_uuid)
    {
        FC_ASSERT( !mail_uuid.empty(), "Mail UUID is empty." );

        // Mail is now fully sent and confirmed by both sides, remove it from storage.
        _app.mail_storage()->remove(mail_uuid);
        // Send notification to other nodes.
        _app.p2p_node()->mail_send_confirm_received(mail_uuid);
    }

    void mail_api::on_mail_confirm_received(const std::string& mail_uuid)
    {
        if(mail_uuid.empty())
        {
            wlog("Mail UUID is empty.");
            return;
        }

        // Mail reception is confirmed by sender, remove it from storage.
        _app.mail_storage()->remove(mail_uuid);
    }
}
