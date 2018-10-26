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

        // Convert array of strings to lowercase.
        template<typename C>
        static C to_lower(const C &container);

        // Create the intersection between two unordered sets.
        // std::set_intersection requires input containers to be sorted, which is not the case with std::unordered_set,
        // so this method reimplements it without such requirement.
        template<typename T>
        static std::unordered_set<T> intersection(const std::unordered_set<T>& a, const std::unordered_set<T>& b);

        // Create a container with copies of elements from 'a' which are not present in 'b'.
        // std::set_difference requires input containers to be sorted, which is not the case with std::unordered_set,
        // so this method reimplements it without such requirement.
        template<typename T>
        static std::unordered_set<T> difference(const std::unordered_set<T>& a, const std::unordered_set<T>& b);

        // Wrappers for corresponding STL functions.
        template<typename C>
        static C set_difference(const C &container1, const C &container2);
        template<typename C>
        static C set_intersection(const C &container1, const C &container2);
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

template<typename C>
C omnibazaar::util::to_lower(const C &container)
{
    C result;
    std::transform(std::begin(container),
                   std::end(container),
                   std::inserter(result, std::end(result)),
                   [](const std::string& str){ return fc::to_lower(str); });
    return result;
}

template<typename T>
std::unordered_set<T> omnibazaar::util::intersection(const std::unordered_set<T>& a, const std::unordered_set<T>& b)
{
    if(a.size() <= b.size())
    {
        std::unordered_set<T> result;
        for(const auto &elem: a)
        {
            if(b.count(elem) > 0)
            {
                result.insert(elem);
            }
        }
        return result;
    }
    else
    {
        return intersection(b, a);
    }
}

template<typename T>
std::unordered_set<T> omnibazaar::util::difference(const std::unordered_set<T>& a, const std::unordered_set<T>& b)
{
    std::unordered_set<T> result;
    for(const auto &elem: a)
    {
        if(b.count(elem) <= 0)
        {
            result.insert(elem);
        }
    }
    return result;
}

template<typename C>
C omnibazaar::util::set_difference(const C &container1, const C &container2)
{
    C result;
    std::set_difference(std::begin(container1), std::end(container1),
                        std::begin(container2), std::end(container2),
                        std::inserter(result, std::end(result)));
    return result;
}

template<typename C>
C omnibazaar::util::set_intersection(const C &container1, const C &container2)
{
    C result;
    std::set_intersection(std::begin(container1), std::end(container1),
                          std::begin(container2), std::end(container2),
                          std::inserter(result, std::end(result)));
    return result;
}
