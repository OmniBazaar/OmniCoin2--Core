#include <mail_storage.hpp>
#include <mail_object.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/scoped_lock.hpp>

static const std::string MAIL_DIR_NAME("mails");
static const std::string TXT_EXTENSION(".txt");

namespace omnibazaar {

    mail_storage::mail_storage(const fc::path& parent_dir)
    {
        set_dir(parent_dir);
    }

    void mail_storage::set_dir(const fc::path& parent_dir)
    {
        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        _parent_dir = parent_dir / MAIL_DIR_NAME;

        // Since root directory is now changed, need to reload the cache.
        reload_cache();
    }

    void mail_storage::reload_cache()
    {
        // Clear cache.
        _cache_by_uuid.clear();
        _cache_by_receiver.clear();

        if(!fc::exists(_parent_dir) || !fc::is_directory(_parent_dir))
            return;

        // Load mail info from disk to cache.
        for (fc::directory_iterator itr(_parent_dir); itr != fc::directory_iterator(); ++itr)
        {
            if (!itr->filename().string().empty() && fc::is_regular_file(*itr))
            {
                mail_object new_mail_object;
                new_mail_object.read_from_file(*itr);
                _cache_by_uuid[new_mail_object.uuid] = mail_info(new_mail_object.recipient);
                _cache_by_receiver.insert( {new_mail_object.recipient, new_mail_object.uuid} );
            }
        }
    }

    void mail_storage::store(const mail_object& mail)
    {
        if(mail.uuid.empty())
        {
            wlog("Mail UUID is empty: ${mail}.", ("mail", mail));
            return;
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        if(_parent_dir.string().empty())
        {
            wlog("Mail parent directory is empty, unable to save mails.");
            return;
        }

        // Save to disk.
        fc::create_directories(_parent_dir);
        mail.write_to_file(_parent_dir / (mail.uuid + TXT_EXTENSION));

        // Save to cache.
        _cache_by_uuid[mail.uuid] = mail_info(mail.recipient);
        _cache_by_receiver.insert( {mail.recipient, mail.uuid} );
    }

    void mail_storage::remove(const std::string& mail_uuid)
    {
        if(mail_uuid.empty())
        {
            wlog("Unable to remove mail because specified UUID is empty.");
            return;
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        // Remove from disk.
        const fc::path mail_path = _parent_dir / (mail_uuid + TXT_EXTENSION);
        if(fc::exists(mail_path))
        {
            fc::remove(mail_path);
        }

        // Remove from cache.
        const auto itr = _cache_by_uuid.find(mail_uuid);
        if(itr != _cache_by_uuid.end())
        {
            // Remove from receiver cache.
            typedef std::unordered_multimap<std::string, std::string>::iterator iter_type;
            // Find all values for specified key.
            std::pair<iter_type, iter_type> itrs = _cache_by_receiver.equal_range(itr->second.receiver);
            // Iterate through values and remove one that matches specified UUID.
            while(itrs.first != itrs.second)
            {
                if((*itrs.first).second == mail_uuid)
                {
                    _cache_by_receiver.erase(itrs.first);
                    break;
                }
                ++itrs.first;
            }

            // Remove from UUID cache.
            _cache_by_uuid.erase(itr);
        }
    }

}
