#pragma once

#include <fc/string.hpp>
#include <fc/filesystem.hpp>
#include <fc/log/logger.hpp>
#include <vector>

namespace omnibazaar {

    // Class for various OmniBazaar-related utility functions.
    class util
    {
    public:
        // Get this computer HDD id.
        static fc::string get_harddrive_id();
        // Get this computer MAC address.
        static fc::string get_primary_mac();
        // Get names of files in specified folder.
        static std::vector<std::string> get_files_in_folder(const std::string &folder);
        // Get file paths in specified folder.
        static std::vector<fc::path> get_files_in_folder(const fc::path& path);
    };
}

// Wrappers over default log macros to use for different logical parts of the project.
#define mail_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("mail"), FORMAT, __VA_ARGS__ )
#define mail_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("mail"), FORMAT, __VA_ARGS__ )
#define mail_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("mail"), FORMAT, __VA_ARGS__ )
#define mail_elog( FORMAT, ... ) fc_elog( fc::logger::get("mail"), FORMAT, __VA_ARGS__ )
#define mail_ddump( SEQ ) mail_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define mail_idump( SEQ ) mail_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define mail_wdump( SEQ ) mail_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define mail_edump( SEQ ) mail_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )

#define bonus_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("bonus"), FORMAT, __VA_ARGS__ )
#define bonus_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("bonus"), FORMAT, __VA_ARGS__ )
#define bonus_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("bonus"), FORMAT, __VA_ARGS__ )
#define bonus_elog( FORMAT, ... ) fc_elog( fc::logger::get("bonus"), FORMAT, __VA_ARGS__ )
#define bonus_ddump( SEQ ) bonus_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define bonus_idump( SEQ ) bonus_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define bonus_wdump( SEQ ) bonus_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define bonus_edump( SEQ ) bonus_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
