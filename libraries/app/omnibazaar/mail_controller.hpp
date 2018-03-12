#pragma once

#include <fc/signals.hpp>
#include <fc/thread/thread.hpp>
#include <fc/thread/future.hpp>

namespace graphene { namespace app {
    class application;
}}

namespace omnibazaar {

    class mail_object;

    // Class for persistent mail management on the node.
    class mail_controller : public std::enable_shared_from_this<mail_controller>
    {
    public:
        // Mail callback type. Exact argument type will depend on callback context.
        typedef std::function<void(fc::variant)> callback_type;

        mail_controller(graphene::app::application& app);
        ~mail_controller();

        // See omnibazaar::mail_api class for descriptions of following methods.
        void send(callback_type cb, const mail_object& mail);
        void subscribe(callback_type cb, const std::string& receiver_name);
        void set_received(const std::string& mail_uuid);
        void confirm_received(const std::string& mail_uuid);

        // Unregister specified callbacks.
        void remove_send_callbacks(const std::unordered_map<std::string, callback_type>& callbacks);
        void remove_receive_callbacks(const std::unordered_map<std::string, callback_type>& callbacks);

    private:
        // Triggered when new mail is received from other users.
        void on_new_mail(const mail_object& mail);
        // Triggered when receive notification is received for specified mail.
        void on_mail_received(const std::string& mail_uuid);
        // Triggered when reception is confirmed by sender.
        void on_mail_confirm_received(const std::string& mail_uuid);

        // Start background thread that continuously tries to re-send pending messages
        // and notifications about message delivery.
        void start_mail_processing_loop();
        void mail_sending_tick();

        void exec_callback(callback_type callback, const std::vector<fc::variant>& objects);

        graphene::app::application& _app;
        fc::thread _thread;
        std::unordered_map<std::string, callback_type> _send_callbacks;
        std::unordered_map<std::string, callback_type> _receive_callbacks;
        boost::signals2::scoped_connection _new_mail_connection;
        boost::signals2::scoped_connection _received_mail_connection;
        boost::signals2::scoped_connection _confirm_received_mail_connection;
    };

}
