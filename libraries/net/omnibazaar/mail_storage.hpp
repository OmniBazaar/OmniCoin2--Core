#pragma once

#include <fc/filesystem.hpp>
#include <fc/thread/mutex.hpp>
#include <unordered_map>

namespace omnibazaar {
    class mail_object;

    // Class for managing storage on disk and access to user mails. This class is thread-safe.
    class mail_storage
    {
    public:
        mail_storage(const fc::path& parent_dir = fc::path());

        // Set parent directory for mail files storage. This is usually the node config parent directory.
        // This method implicitly calls reload_cache().
        void set_dir(const fc::path& parent_dir);

        // Store specified mail object to folder with undelivered mail.
        void store(const mail_object& mail);

        // Move specified mail to delivered folder.
        void set_received(const std::string& mail_uuid);

        // Remove mail with specified UUID from cache and disk.
        void remove(const std::string& mail_uuid);

        // Get all pending mails for specified receiver.
        std::vector<mail_object> get_mails_by_receiver(const std::string& receiver)const;

        // Get all mails that are not yet delivered to receivers.
        std::vector<fc::path> get_pending_mails()const;

        // Get all mails UUIDs which are confirmed by receivers.
        std::vector<std::string> get_received_mails()const;

    private:
        // Helper struct for cache.
        struct mail_info
        {
            // Message receiver.
            std::string receiver;
            // Flag indicating that receiver notified node that message was delivered,
            // message is now placed to "delivered" folder, and waiting for sender to get delivery notification.
            bool is_delivered;

            mail_info(const std::string& r = std::string(), const bool d = false)
                : receiver(r), is_delivered(d)
            {}
        };

        // Reset and reload mail cache from disk.
        void reload_cache();

        std::unordered_map<std::string, mail_info> _cache_by_uuid;
        std::unordered_multimap<std::string, std::string> _cache_by_receiver;
        fc::path _parent_dir;
        mutable fc::mutex _mutex;
    };
}
