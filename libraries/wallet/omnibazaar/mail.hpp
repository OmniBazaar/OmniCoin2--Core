#pragma once

#include <string>
#include <vector>

// Forward declaration for wallet class.
namespace graphene { namespace wallet { class wallet_api; } }

namespace omnibazaar {

    // Class for managing user communication system.
    class mail
    {
    public:
        // Mail requires a wallet instance.
        mail(graphene::wallet::wallet_api &w);

        // Perform different mail management actions. Parameters depend on action types.
        // See implementation for supported action types and parameters.
        std::vector<std::string> mail_service(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);

    private:
        graphene::wallet::wallet_api &wallet;

        static int mail_find_latest(const std::vector<std::string> &mail_data);
        static std::vector<std::string> mail_sort_datewise(const std::vector<std::string> &mail_data);
        static std::vector<std::string> read_mail(const std::string &folder, const std::string &mail_file);

        std::string get_mail_dir()const;

        std::vector<std::string> mail_service_save(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
        std::vector<std::string> mail_service_get(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
        std::vector<std::string> mail_service_delete(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
        std::vector<std::string> mail_service_mark_as_read(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
        std::vector<std::string> mail_service_add_to_list(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
        std::vector<std::string> mail_service_get_unread_mail_status(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3);
    };
}
