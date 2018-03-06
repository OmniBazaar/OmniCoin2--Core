#include "mail_api.h"
#include <mail_object.hpp>
#include <graphene/app/application.hpp>

namespace omnibazaar {

    mail_api::mail_api(graphene::app::application &a)
        : _app(a)
    {
    }

    void mail_api::send(callback_type cb, const mail_object& mail)
    {
        FC_ASSERT( !mail.uuid.empty(), "Mail UUID is empty" );

        _send_callbacks[mail.uuid] = cb;
        _app.p2p_node()->store_undelivered_mail(mail);
    }

    void mail_api::subscribe(callback_type cb, const std::string& receiver_name)
    {
        FC_ASSERT( !receiver_name.empty(), "Mail receiver name cannot be empty." );

        _receive_callbacks[receiver_name] = cb;
    }

    void mail_api::set_received(const std::string& mail_uuid)
    {
        FC_ASSERT( !mail_uuid.empty(), "Mail UUID is empty." );

        // TODO: send receive notification.
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

        // Need to ensure the object is not deleted for the life of the async operation.
        auto capture_this = shared_from_this();

        // Invoke the callback.
        auto callback = iter->second;
        fc::async( [capture_this, this, mail, callback](){
           callback( fc::variant(mail) );
        } );
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

        // Need to ensure the object is not deleted for the life of the async operation.
        auto capture_this = shared_from_this();

        // Invoke the callback.
        auto callback = iter->second;
        fc::async( [capture_this, this, mail_uuid, callback](){
           callback( fc::variant(send_confirmation{mail_uuid}) );
        } );
    }
}
