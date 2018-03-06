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

        // Store specified mail object.
        void store(const mail_object& mail);

        // Remove mail with specified UUID from cache and disk.
        void remove(const std::string& mail_uuid);

    private:
        // Helper struct for cache.
        struct mail_info
        {
            std::string receiver;

            mail_info(const std::string& r = std::string())
                : receiver(r)
            {}
        };

        // Reset and reload mail cache from disk.
        void reload_cache();

        std::unordered_map<std::string, mail_info> _cache_by_uuid;
        std::unordered_multimap<std::string, std::string> _cache_by_receiver;
        fc::path _parent_dir;
        fc::mutex _mutex;
    };
}
