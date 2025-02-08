module;
/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2018-2025 Alexander Diemand

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "date/date.h"

#include <filesystem>

#ifndef _WIN32
#include <cstring>
#include <pwd.h>
#if __has_include(<sys/types.h>)
#  include <sys/types.h>
#endif
#if __has_include(<unistd.h>)
#  include <unistd.h>
#endif
#else
#include <windows.h>
#include <sysinfoapi.h>
#endif


module lxr_os;


namespace lxr {

const std::string OS::hostname() noexcept
{
#if defined( __linux__ ) || defined( __APPLE__ ) || defined(__FreeBSD__)
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    return hostname;
#else
    #ifdef _WIN32
    TCHAR buf[512] = ""; DWORD bufsz = 511;
    std::string _hostname {"win"};
    if (GetComputerNameEx(ComputerNameDnsDomain, (LPSTR)&buf, &bufsz)) {
      _hostname = std::string(buf, bufsz);
    }
    return _hostname;
    #else
    #error Do not know how to handle this!
    #endif
#endif
}

const std::string OS::username() noexcept
{
#if defined( __linux__ ) || defined( __APPLE__ ) || defined(__FreeBSD__)
    constexpr int blen = 65536;
    char buffer[blen];
    char username[1024];
    uid_t uid = geteuid();
    struct passwd pwd, *res;
    int retval;
    if ((retval = getpwuid_r(uid, &pwd, buffer, blen, &res)) == 0) {
        strncpy(username, pwd.pw_name, 1023);
    } else {
        strncpy(username, "unknown\0\0", 1023);
    }
    return username;
#else
    #ifdef _WIN32
    TCHAR buf[512] = ""; DWORD bufsz = 511;
    std::string _username {"user"};
    if (GetUserNameA((LPSTR)&buf, &bufsz)) {
      _username = std::string(buf, bufsz);
    }
    return _username;
    #else
    #error Do not know how to handle this!
    #endif
#endif

}

template <typename Clock>
std::time_t to_time_t(Clock cl) {
    using namespace std::chrono;
    auto casted = time_point_cast<system_clock::duration>(cl - Clock::clock::now() + system_clock::now());
    return system_clock::to_time_t(casted);
}

const std::string OS::timestamp() noexcept
{
    auto now = std::chrono::system_clock::now();
    return date::format("%Y%m%dT%H%M%S", date::floor<std::chrono::seconds>(now));
    // return std::format("%Y%m%dT%H%M%S", date::floor<std::chrono::seconds>(now));
}

const std::string OS::time2string(time_t _t) noexcept
{
    auto ts = std::chrono::system_clock::from_time_t(_t);
    return date::format("%Y%m%dT%H%M%S", date::floor<std::chrono::seconds>(ts));
}

const std::string OS::time2string(std::filesystem::file_time_type ts) noexcept
{
    const auto deltat = ts - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now();
    const auto syst = std::chrono::time_point_cast<std::chrono::system_clock::duration>(deltat);
    return time2string(std::chrono::system_clock::to_time_t(syst));

}

} // namespace

// formatted output of std::chrono::microseconds
std::ostream&
operator<<(std::ostream& os, std::chrono::microseconds const &us) {
    os << us.count() << " µs";
    return os;
}
