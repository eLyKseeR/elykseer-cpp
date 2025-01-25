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


#include <filesystem>
#include <optional>
#include <sstream>

#if !defined(PLATFORM_win64)
#include <sys/types.h>
#include <sys/stat.h>
#endif

import lxr_sha3;

import lxr_os;

module lxr_coqfilesupport;


namespace lxr {

typedef std::string filename;

std::optional<CoqFilesupport::FileInformation> CoqFilesupport::get_file_information(const filename &fn) noexcept
{
    FileInformation fi;

    std::filesystem::path fp{fn};
    if (! std::filesystem::exists(fp)) { return {}; }

    fi.fname = fn;
    fi.fchecksum = lxr::Sha3_256::hash(fp).toHex();

    #if defined(PLATFORM_darwin) || defined(PLATFORM_freebsd)
    #define ST_SECONDS st_mtimespec.tv_sec
    #define S_STAT stat
    #define F_STAT stat
    #elif defined(PLATFORM_linux)
    #define ST_SECONDS st_mtim.tv_sec
    #define S_STAT stat
    #define F_STAT stat
    #elif defined(PLATFORM_win64)
    #define ST_SECONDS st_mtime
    #define S_STAT _stat64
    #define F_STAT _wstat64
    #else
    #error not yet defined for this platform (PLATFORM)
    #endif
    {
        struct S_STAT _fstats{};
        if (F_STAT(fp.c_str(), &_fstats) == 0) {
            fi.fname = fp.string();
            fi.fchecksum = lxr::Sha3_256::hash(fp).toHex();
            std::ostringstream ss;
            ss << _fstats.st_uid;
            fi.fowner = ss.str();
            fi.fsize = (int64_t)_fstats.st_size;
            fi.fmodified = OS::time2string(_fstats.ST_SECONDS);
            fi.fpermissions = 100 * ((_fstats.st_mode >> 6) & 0x007) + 10 * ((_fstats.st_mode >> 3)  & 0x007) + (_fstats.st_mode & 0x007);
        } else {
            // std::clog << "fstats failed for " << fp << ": " << std::strerror(errno) << std::endl;
            return {};
        }
    }
    return std::move(fi);
}

} // namespace