#include <mail.hpp>
#include <omnibazaar_util.hpp>
#include <graphene/wallet/wallet.hpp>
#include <fc/filesystem.hpp>

namespace omnibazaar
{
    namespace constants
    {
        static const std::string mail_dir("mail");
        static const std::string mail_outbox("/outbox");
        static const std::string mail_inbox("/inbox");
        static const std::string mail_sent("/sent");
        static const std::string mail_deleted("/deleted");
        static const std::string mail_data_file("/mail.set");
    }

    mail::mail(graphene::wallet::wallet_api &w)
        : wallet(w)
    {
    }

    int mail::mail_find_latest(const std::vector<std::string> &mail_data)
    {
        int i, k = -1;
        long mDate = 0;
        for (i = 0; i < mail_data.size(); i += 8)
        {
            long t = atol(mail_data[i].c_str());
            if (mDate < t)
            {
                mDate = t; k = i;
            }
        }
        if (mDate == 0)
        {
            return -1;
        }
        return k;
    }

    std::vector<std::string> mail::mail_sort_datewise(const std::vector<std::string> &mail_data)
    {
        std::vector<std::string> sortedMails;
        std::vector<std::string> mail_data_tmp = mail_data;
        int i, k = 1;
        while (k != -1)
        {
            k = mail_find_latest(mail_data_tmp);
            if (k == -1)
            {
                break;
            }
            for (i = 0; i < 8; i++)
            {
                sortedMails.push_back(mail_data_tmp[k + i]);
            }
            mail_data_tmp[k] = "0";
        }
        return sortedMails;
    }

    std::vector<std::string> mail::read_mail(const std::string &folder, const std::string &mail_file)
    {
        std::string full_path = folder;
        std::string mail_filename = mail_file;

        if (mail_file.length() > 0)
        {
            full_path += "/";
            full_path += mail_file;
        }
        else
        {
            const size_t pos = full_path.find_last_of("/");
            mail_filename = full_path.substr(pos + 1);
        }

        std::ifstream fs;
        std::string message;
        fs.open(full_path);
        fs >> message;
        fs.close();

        std::vector<std::string> mymail;
        std::istringstream f1(message);
        std::string s;
        while (getline(f1, s, '^'))
        {
            mymail.push_back(s);
        }
        while (mymail.size() < 6)
        {
            mymail.push_back("");
        }

        mymail[2] = fc::base64_decode(mymail[2]);
        mymail[3] = fc::base64_decode(mymail[3]);

        std::vector<std::string> return_data;
        return_data.push_back(mail_filename);
        return_data.push_back(mymail[0]);   // sender
        return_data.push_back(mymail[1]);   // receipent
        return_data.push_back(mymail[2]);   // subject
        return_data.push_back(mymail[3]);   // body
        return_data.push_back(mymail[4]);   // read_status

        time_t t = atol(mymail[5].c_str()); // create date
        struct tm *tm = localtime(&t);
        char date[100];
        sprintf(date, "%d-%d-%d %d:%d:%d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        return_data.push_back(date);

        if (mymail.size() >= 7)
        {
            t = atol(mymail[6].c_str());    // received date
            tm = localtime(&t);
            sprintf(date, "%d-%d-%d %d:%d:%d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            return_data.push_back(date);
        }
        else
        {
            return_data.push_back("");
        }

        return return_data;
    }

    std::string mail::get_mail_dir()const
    {
        return (fc::path(wallet.get_wallet_filename()).parent_path() / constants::mail_dir).string();
    }

    std::vector<std::string> mail::mail_service(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        if (action == "save")
        {
            return mail_service_save(action, param1, param2, param3);
        }
        else if (action == "get")
        {
            return mail_service_get(action, param1, param2, param3);
        }
        else if (action == "delete")
        {
            return mail_service_delete(action, param1, param2, param3);
        }
        else if (action == "mark_as_read")
        {
            return mail_service_mark_as_read(action, param1, param2, param3);
        }
        else if (action == "add_to_list")
        {
            return mail_service_add_to_list(action, param1, param2, param3);
        }
        else if (action == "get_unread_mail_status")
        {
            return mail_service_get_unread_mail_status(action, param1, param2, param3);
        }

        return std::vector<std::string>();
    }

    std::vector<std::string> mail::mail_service_save(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        vector<std::string> return_data;

        const std::string mail_dir = get_mail_dir();
        const std::string mail_outbox = mail_dir + constants::mail_outbox;
        if (!fc::exists(mail_dir)) fc::create_directories(mail_dir);
        if (!fc::exists(mail_outbox)) fc::create_directories(mail_outbox);

        std::vector<std::string> to;
        std::istringstream f(param1);
        std::string s;
        while (getline(f, s, ','))
        {
            s.erase(remove_if(s.begin(), s.end(), [](const char c) { return isspace(c); }), s.end());
            to.push_back(s);
        }

        const auto accounts = wallet.list_my_accounts();

        for (size_t i = 0; i < to.size(); i++)
        {
            string mailBox = mail_outbox + "/" + to[i];
            if (!fc::exists(mailBox)) fc::create_directories(mailBox);

            char fullpath[255];
            sprintf(fullpath, "%s/%ld.mail", mailBox.c_str(), time(0));

            string message;
            message += accounts[0].name;                                          // sender
            message += "^" + to[i];                                               // receipent
            message += "^" + fc::base64_encode(param2.c_str(), param2.length());  // subject
            message += "^" + fc::base64_encode(param3.c_str(), param3.length());  // body
            message += "^0";                                                      // read status
            message += "^" + std::to_string(time(0));                             // created_time

            std::ofstream fs;
            fs.open(string(fullpath));
            fs << message;
            fs.close();
        }

        return return_data;
    }

    std::vector<std::string> mail::mail_service_get(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        vector<std::string> return_data;

        const std::string mail_dir = get_mail_dir();
        const std::string mail_outbox = mail_dir + constants::mail_outbox;
        const std::string mail_inbox = mail_dir + constants::mail_inbox;
        const std::string mail_sent = mail_dir + constants::mail_sent;
        const std::string mail_deleted = mail_dir + constants::mail_deleted;
        if (!fc::exists(mail_dir)) fc::create_directories(mail_dir);
        if (!fc::exists(mail_outbox)) fc::create_directories(mail_outbox);
        if (!fc::exists(mail_inbox)) fc::create_directories(mail_inbox);
        if (!fc::exists(mail_sent)) fc::create_directories(mail_sent);
        if (!fc::exists(mail_deleted)) fc::create_directories(mail_deleted);

        string folder = mail_inbox;
        if (param1 == "my_name")
        {
            auto accounts = wallet.list_my_accounts();
            return_data.push_back(accounts[0].name);
            return return_data;
        }

        if      (param1 == "inbox")     folder = mail_inbox;
        else if (param1 == "sent")      folder = mail_sent;
        else if (param1 == "deleted")   folder = mail_deleted;
        else if (param1 == "outbox")    folder = mail_outbox;
        fc::path p(folder);

        int readMailCount = 0;
        int totalMailCount = 0;
        int i;

        if (is_directory(p))
        {
            for (fc::directory_iterator itr(p); itr != fc::directory_iterator(); ++itr)
            {
                if (param1 == "outbox")
                {
                    if (!itr->stem().string().empty() && fc::is_directory(*itr))
                    {
                        string sendOwner = (*itr).stem().string();
                        std::vector<std::string> list = util::get_files_in_folder(mail_outbox + "/" + sendOwner);
                        for (i = 0; i < list.size(); i++)
                        {
                            std::vector<std::string> mymail = read_mail(list[i], "");
                            for (int j = 0; j < mymail.size(); j++)
                            {
                                return_data.push_back(mymail[j]);
                            }
                        }
                    }
                }
                else
                {
                    if (!itr->filename().string().empty() && fc::is_regular_file(*itr))
                    {
                        string mailFile = itr->filename().string();
                        std::vector<std::string> mymail = read_mail(folder, mailFile);
                        totalMailCount++;
                        if (mymail[5] == "1")
                        {
                            readMailCount++;
                        }
                        for (i = 0; i < mymail.size(); i++)
                        {
                            return_data.push_back(mymail[i]);
                        }
                    }
                }
            }

            if (param1 == "inbox")
            {
                auto path = get_mail_dir() + constants::mail_data_file;
                FILE *fl = fopen(path.c_str(), "w");
                fprintf(fl, "%d|%d", readMailCount, totalMailCount);
                fclose(fl);
            }

            // lets sort mails date wise
            return_data = mail_sort_datewise(return_data);
        }

        return return_data;
    }

    std::vector<std::string> mail::mail_service_delete(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        std::vector<string> return_data;

        const std::string mail_dir = get_mail_dir();
        const std::string mail_outbox = mail_dir + constants::mail_outbox;
        const std::string mail_inbox = mail_dir + constants::mail_inbox;
        const std::string mail_sent = mail_dir + constants::mail_sent;
        const std::string mail_deleted = mail_dir + constants::mail_deleted;
        if (!fc::exists(mail_dir)) fc::create_directories(mail_dir);
        if (!fc::exists(mail_outbox)) fc::create_directories(mail_outbox);
        if (!fc::exists(mail_inbox)) fc::create_directories(mail_inbox);
        if (!fc::exists(mail_sent)) fc::create_directories(mail_sent);
        if (!fc::exists(mail_deleted)) fc::create_directories(mail_deleted);

        if (param1 == "deleted")
        {
            string f = mail_deleted + "/" + param2;
            if (fc::exists(f))
            {
                fc::remove(f.c_str());
            }
        }
        else
        {
            string folder = mail_inbox;
            if      (param1 == "inbox")     folder = mail_inbox;
            else if (param1 == "sent")      folder = mail_sent;
            else if (param1 == "outbox")    folder = mail_outbox;

            string f1 = folder + "/" + param2;
            string f2 = mail_deleted + "/" + param2;
            if (param1 == "outbox")
            {
                size_t pos = param2.find('/');
                f2 = mail_deleted + "/" + param2.substr(pos+1);
            }
            fc::copy(f1, f2);
            fc::remove(f1.c_str());
        }

        return return_data;
    }

    std::vector<std::string> mail::mail_service_mark_as_read(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        std::vector<std::string> return_data;

        const std::string mail_dir = get_mail_dir();
        const std::string mail_outbox = mail_dir + constants::mail_outbox;
        const std::string mail_inbox = mail_dir + constants::mail_inbox;
        const std::string mail_sent = mail_dir + constants::mail_sent;
        if (!fc::exists(mail_dir)) fc::create_directories(mail_dir);
        if (!fc::exists(mail_outbox)) fc::create_directories(mail_outbox);
        if (!fc::exists(mail_inbox)) fc::create_directories(mail_inbox);
        if (!fc::exists(mail_sent)) fc::create_directories(mail_sent);

        string folder;
        if      (param1 == "inbox")     folder = mail_inbox;
        else if (param1 == "sent")      folder = mail_sent;
        else if (param1 == "outbox")    folder = mail_outbox;

        if (folder != "")
        {
            string fullPath = folder + "/" + param2;

            string message;
            std::ifstream fs;
            fs.open(fullPath);
            fs >> message;
            fs.close();

            vector<string> mymail;
            std::istringstream f1(message);
            string s;
            while (getline(f1, s, '^'))
            {
                mymail.push_back(s);
            }
            mymail[4] = "1";

            std::ofstream fs1;
            fs1.open(fullPath);
            fs1 << mymail[0] + "^" + mymail[1] + "^" + mymail[2] + "^" + mymail[3] + "^" + mymail[4] + "^" + mymail[5] + "^" + mymail[6];
            fs1.close();
        }

        return return_data;
    }

    std::vector<std::string> mail::mail_service_add_to_list(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        std::vector<std::string> return_data;

        if (param1 != "")
        {
            // TODO: migrate P2P mail code.
            //_p2p_node->mail_sendTo(param1);
        }

        return return_data;
    }

    std::vector<std::string> mail::mail_service_get_unread_mail_status(const std::string &action, const std::string &param1, const std::string &param2, const std::string &param3)
    {
        std::vector<std::string> return_data;

        int readMailCount = 0, totalMailCount = 0;

        auto path = get_mail_dir() + constants::mail_data_file;
        FILE *fl = fopen(path.c_str(), "r");
        if (fl != 0)
        {
            fscanf(fl, "%d|%d", &readMailCount, &totalMailCount);
            fclose(fl);
        }

        if (readMailCount != totalMailCount)
        {
            return_data.push_back("1");
        }
        else
        {
            return_data.push_back("0");
        }

        return return_data;
    }

}
