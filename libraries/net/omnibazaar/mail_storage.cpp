#include <mail_storage.hpp>
#include <mail_object.hpp>
#include <omnibazaar_util.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/io/json.hpp>

static const std::string DELIVERED_STR("delivered");
static const std::string UNDELIVERED_STR("undelivered");
// One folder is for storing mail that was sent but not yet flagged as received,
// another folder is for storing mail for which sender did not yet get delivery notification.
static const std::pair<std::string, bool> MAIL_FOLDERS[2] = {
    { DELIVERED_STR, true },
    { UNDELIVERED_STR, false}
};

namespace omnibazaar {

    mail_storage::mail_storage(const fc::path& parent_dir)
    {
        mail_ddump((parent_dir));

        set_dir(parent_dir);
    }

    void mail_storage::set_dir(const fc::path& parent_dir)
    {
        mail_ddump((parent_dir));

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        mail_ilog("Changing mail directory from '${olddir}' to '${newdir}'.", ("olddir", _parent_dir)("newdir", parent_dir));

        _parent_dir = parent_dir;

        // Since root directory is now changed, need to reload the cache.
        reload_cache();
    }

    void mail_storage::reload_cache()
    {
        mail_ddump((""));

        // Clear cache.
        _cache_by_uuid.clear();
        _cache_by_receiver.clear();

        if(!fc::exists(_parent_dir) || !fc::is_directory(_parent_dir))
        {
            mail_wlog("Mail parent directory '${dir}' does not exist or is not a directory.", ("dir", _parent_dir));
            return;
        }

        for(auto folder_info : MAIL_FOLDERS)
        {
            const fc::path path = _parent_dir / folder_info.first;
            if(!fc::exists(path))
            {
                mail_wlog("Mail folder '${path}' does not exist.", ("path", path));
                continue;
            }

            // Load mail info from disk to cache.
            for (fc::directory_iterator itr(path); itr != fc::directory_iterator(); ++itr)
            {
                mail_dlog("Loading mail from '${path}'", ("path", *itr));
                if (!itr->string().empty() && fc::is_regular_file(*itr))
                {
                    const fc::variant var = fc::json::from_file(*itr);
                    mail_dlog("Loaded mail variable: ${var}", ("var", var));
                    if(!var.is_null())
                    {
                        const mail_object new_mail_object = var.as<mail_object>();
                        mail_dlog("Loaded mail object: ${obj}", ("obj", new_mail_object));
                        _cache_by_uuid[new_mail_object.uuid] = mail_info(new_mail_object.recipient, folder_info.second);
                        _cache_by_receiver.insert( {new_mail_object.recipient, new_mail_object.uuid} );
                    }
                }
            }
        }
    }

    void mail_storage::store(const mail_object& mail)
    {
        mail_ddump((mail));

        if(mail.uuid.empty())
        {
            mail_wlog("Mail UUID is empty: ${mail}.", ("mail", mail));
            return;
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        if(_parent_dir.string().empty())
        {
            mail_wlog("Mail parent directory is empty, unable to save mails.");
            return;
        }

        // Save to disk.
        fc::create_directories(_parent_dir / UNDELIVERED_STR);
        fc::json::save_to_file(mail, _parent_dir / UNDELIVERED_STR / mail.uuid);

        // Save to cache.
        _cache_by_uuid[mail.uuid] = mail_info(mail.recipient, false);
        _cache_by_receiver.insert( {mail.recipient, mail.uuid} );
    }

    void mail_storage::remove(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        if(mail_uuid.empty())
        {
            mail_wlog("Unable to remove mail because specified UUID is empty.");
            return;
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        // Remove from disk.
        mail_dlog("Removing '${mail}' from disk.", ("mail", mail_uuid));
        for(auto folder_info : MAIL_FOLDERS)
        {
            const fc::path mail_path = _parent_dir / folder_info.first / mail_uuid;
            if(fc::exists(mail_path))
            {
                fc::remove(mail_path);
            }
            else
            {
                mail_wlog("Path '${path}' does not exist.", ("path", mail_path));
            }
        }

        // Remove from cache.
        mail_dlog("Removing '${mail}' from cache.", ("mail", mail_uuid));
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

    std::vector<mail_object> mail_storage::get_mails_by_receiver(const std::string& receiver)const
    {
        mail_ddump((receiver));

        if(receiver.empty())
        {
            mail_wlog("Receiver name is empty.");
            return std::vector<mail_object>();
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        if(!fc::exists(_parent_dir))
        {
            mail_wlog("Mails directory '${dir}' does not exist.", ("dir", _parent_dir));
            return std::vector<mail_object>();
        }

        // Read and return mail.
        std::vector<mail_object> mails;
        typedef std::unordered_multimap<std::string, std::string>::const_iterator iter_type;
        std::pair<iter_type, iter_type> itrs = _cache_by_receiver.equal_range(receiver);
        while(itrs.first != itrs.second)
        {
            const fc::path mail_path = _parent_dir / UNDELIVERED_STR / (*itrs.first).second;
            if(fc::exists(mail_path))
            {
                const fc::variant var = fc::json::from_file(mail_path);
                mail_ddump((var));
                if(!var.is_null())
                {
                    mails.push_back(var.as<mail_object>());
                }
                else
                {
                    mail_wlog("Unable to read mail from '${path}'", ("path", mail_path));
                }
            }
            else
            {
                mail_wlog("File '${path}' does not exist.", ("path", mail_path));
            }

            ++itrs.first;
        }
        return mails;
    }

    void mail_storage::set_received(const std::string& mail_uuid)
    {
        mail_ddump((mail_uuid));

        if(mail_uuid.empty())
        {
            mail_wlog("Mail UUID is empty.");
            return;
        }

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        if(!fc::exists(_parent_dir / UNDELIVERED_STR))
        {
            mail_wlog("Mails directory '${path}' does not exist.", ("path", _parent_dir / UNDELIVERED_STR));
            return;
        }

        // Update value in cache.
        mail_dlog("Updating cache.");
        auto itr = _cache_by_uuid.find(mail_uuid);
        if(itr != _cache_by_uuid.end())
        {
            itr->second.is_delivered = true;
        }

        // Move mail file to delivered folder.
        const fc::path current_mail_path = _parent_dir / UNDELIVERED_STR / mail_uuid;
        mail_dlog("Moving file '${file}' to delivered folder '${dir}'", ("file", current_mail_path)("dir", _parent_dir / DELIVERED_STR));
        if(fc::exists(current_mail_path))
        {
            const fc::path new_mail_dir = _parent_dir / DELIVERED_STR;
            const fc::path new_mail_path = new_mail_dir / mail_uuid;

            // create the delivered folder if it doesn't exist
            if (!fc::exists(new_mail_dir))
            {
                mail_wlog("Directory '${dir}' does not exist, creating.", ("dir", new_mail_dir));
                fc::create_directories(new_mail_dir);
            }

            // move the mail file from the undelivered folder to the delivered folder
            mail_dlog("Moving '${path1}' to '${path2}'", ("path1", current_mail_path)("path2", new_mail_path));
            fc::rename(current_mail_path, new_mail_path);
        }
        else
        {
            mail_wlog("File '${path}' does not exist.", ("path", current_mail_path));
        }
    }

    std::vector<fc::path> mail_storage::get_pending_mails()const
    {
        mail_ddump((""));

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        mail_dlog("Getting mails from '${dir}'", ("dir", _parent_dir / UNDELIVERED_STR));
        return util::get_files_in_folder(_parent_dir / UNDELIVERED_STR);
    }

    std::vector<std::string> mail_storage::get_received_mails()const
    {
        mail_ddump((""));

        // Thread safety.
        const fc::scoped_lock<fc::mutex> lock(_mutex);

        std::vector<std::string> ids;
        for(const auto itr : _cache_by_uuid)
        {
            if(itr.second.is_delivered)
            {
                ids.push_back(itr.first);
            }
        }
        mail_ddump((ids));
        return ids;
    }

}
