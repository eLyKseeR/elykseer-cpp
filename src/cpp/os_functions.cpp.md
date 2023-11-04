```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/os.hpp"
#include "date/date.h"

#include <format>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#include <sysinfoapi.h>
#endif

namespace lxr {

````
declared in [OS](os.hpp.md)

```cpp

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
    char username[1024];
    username[1023] = '\0';
    getlogin_r(username, 1023);
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
```

```cpp
} // namespace
```

## formatted output of std::chrono::microseconds
```cpp
std::ostream&
operator<<(std::ostream& os, std::chrono::microseconds const &us) {
    os << us.count() << " µs";
    return os;
}
```
