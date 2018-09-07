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
        // Calculate square root using only integers.
        static uint32_t isqrt(const uint32_t n);
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

#define escrow_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("escrow"), FORMAT, __VA_ARGS__ )
#define escrow_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("escrow"), FORMAT, __VA_ARGS__ )
#define escrow_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("escrow"), FORMAT, __VA_ARGS__ )
#define escrow_elog( FORMAT, ... ) fc_elog( fc::logger::get("escrow"), FORMAT, __VA_ARGS__ )
#define escrow_ddump( SEQ ) escrow_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define escrow_idump( SEQ ) escrow_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define escrow_wdump( SEQ ) escrow_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define escrow_edump( SEQ ) escrow_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )

#define pop_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("pop"), FORMAT, __VA_ARGS__ )
#define pop_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("pop"), FORMAT, __VA_ARGS__ )
#define pop_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("pop"), FORMAT, __VA_ARGS__ )
#define pop_elog( FORMAT, ... ) fc_elog( fc::logger::get("pop"), FORMAT, __VA_ARGS__ )
#define pop_ddump( SEQ ) pop_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define pop_idump( SEQ ) pop_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define pop_wdump( SEQ ) pop_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define pop_edump( SEQ ) pop_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )

#define market_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("market"), FORMAT, __VA_ARGS__ )
#define market_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("market"), FORMAT, __VA_ARGS__ )
#define market_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("market"), FORMAT, __VA_ARGS__ )
#define market_elog( FORMAT, ... ) fc_elog( fc::logger::get("market"), FORMAT, __VA_ARGS__ )
#define market_ddump( SEQ ) market_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define market_idump( SEQ ) market_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define market_wdump( SEQ ) market_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define market_edump( SEQ ) market_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )

#define exchange_dlog( FORMAT, ... ) fc_dlog( fc::logger::get("exchange"), FORMAT, __VA_ARGS__ )
#define exchange_ilog( FORMAT, ... ) fc_ilog( fc::logger::get("exchange"), FORMAT, __VA_ARGS__ )
#define exchange_wlog( FORMAT, ... ) fc_wlog( fc::logger::get("exchange"), FORMAT, __VA_ARGS__ )
#define exchange_elog( FORMAT, ... ) fc_elog( fc::logger::get("exchange"), FORMAT, __VA_ARGS__ )
#define exchange_ddump( SEQ ) exchange_dlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define exchange_idump( SEQ ) exchange_ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define exchange_wdump( SEQ ) exchange_wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
#define exchange_edump( SEQ ) exchange_elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )
