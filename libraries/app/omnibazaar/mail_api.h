#pragma once

#include <fc/api.hpp>

namespace graphene { namespace app {
    class application;
}}

namespace omnibazaar {
    class mail_object;

    // Class for mail system API.
    class mail_api : public std::enable_shared_from_this<mail_api>
    {
    public:
        // Return value for send callback.
        struct send_confirmation
        {
            std::string msg_uuid;
        };

        // Mail callback type. Exact argument type will depend on callback context.
        typedef std::function<void(fc::variant)> callback_type;

        mail_api(graphene::app::application& a);

        // Send specified mail object and subscribe for a callback that triggers when mail object is delivered.
        // Callback argument is of type "send_confirmation".
        void send(callback_type cb, const mail_object& mail);

        // Subscribe for receiving new mail for specified user.
        // Callback argument is of type "mail_object".
        void subscribe(callback_type cb, const std::string& receiver_name);

        // Notify the network that mail with specified UUID was successfully received.
        void set_received(const std::string& mail_uuid);

    private:
        // Triggered when new mail is received from other users.
        void on_new_mail(const mail_object& mail);
        // Triggered when receive notification is received for specified mail.
        void on_mail_received(const std::string& mail_uuid);

        graphene::app::application& _app;
        std::unordered_map<std::string, callback_type> _send_callbacks;
        std::unordered_map<std::string, callback_type> _receive_callbacks;
    };
}

FC_REFLECT( omnibazaar::mail_api::send_confirmation,
            (msg_uuid)
            )

FC_API( omnibazaar::mail_api,
        (send)
        (subscribe)
        (set_received)
        )
