#pragma once

#include <fc/api.hpp>

namespace graphene { namespace app {
    class application;
}}

namespace omnibazaar {
    class mail_object;

    // Class for mail system API.
    class mail_api
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
        ~mail_api();

        // Used by Sender.
        // Send specified mail object and subscribe for a callback that triggers when mail object is delivered.
        // Callback argument is of type "send_confirmation".
        // At this stage Sender has this mail in his Outbox folder.
        void send(callback_type cb, const mail_object& mail);

        // Used by Receiver.
        // Subscribe for receiving new mail for specified user.
        // Callback argument is of type "mail_object".
        // Upon call to this method it also triggers callback for any pending mails for specified receiver.
        // After callback is executed Receiver should see new mail in his Inbox folder.
        void subscribe(callback_type cb, const std::string& receiver_name);

        // Used by Receiver.
        // Notify the network that mail with specified UUID was successfully received.
        // This triggeres a callback for Sender and Sender can move the mail to his Sent folder and call confirm_received().
        void set_received(const std::string& mail_uuid);

        // Used by Sender.
        // Send notification that delivery info was accepted by Sender.
        // After this mail object is deleted from backend.
        void confirm_received(const std::string& mail_uuid);

    private:
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
        (confirm_received)
        )
