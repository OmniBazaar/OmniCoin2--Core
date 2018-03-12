#include <mail_api.h>
#include <mail_object.hpp>
#include <mail_controller.hpp>
#include <graphene/app/application.hpp>

namespace omnibazaar {

    mail_api::mail_api(graphene::app::application &a)
        : _app(a)
    {
    }

    mail_api::~mail_api()
    {
        _app.mail_controller()->remove_send_callbacks(_send_callbacks);
        _app.mail_controller()->remove_receive_callbacks(_receive_callbacks);
    }

    void mail_api::send(callback_type cb, const mail_object& mail)
    {
        FC_ASSERT( !mail.uuid.empty(), "Mail UUID is empty" );

        _send_callbacks[mail.uuid] = cb;
        _app.mail_controller()->send(cb, mail);
    }

    void mail_api::subscribe(callback_type cb, const std::string& receiver_name)
    {
        FC_ASSERT( !receiver_name.empty(), "Mail receiver name cannot be empty." );

        _receive_callbacks[receiver_name] = cb;
        _app.mail_controller()->subscribe(cb, receiver_name);
    }

    void mail_api::set_received(const std::string& mail_uuid)
    {
        FC_ASSERT( !mail_uuid.empty(), "Mail UUID is empty." );

        _app.mail_controller()->set_received(mail_uuid);
    }

    void mail_api::confirm_received(const std::string& mail_uuid)
    {
        FC_ASSERT( !mail_uuid.empty(), "Mail UUID is empty." );

        _app.mail_controller()->confirm_received(mail_uuid);
    }

}
