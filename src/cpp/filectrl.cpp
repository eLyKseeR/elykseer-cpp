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

#include "generator.hpp"

#include <filesystem>
typedef std::filesystem::path filepath;

#include <optional>
#include <vector>


#if __has_include(<unistd.h>)
#  include <unistd.h>
#endif
#if __has_include(<sys/stat.h>)
#  include <sys/stat.h>
#endif


module lxr_filectrl;


namespace lxr {

static uid_t fp_uid = getuid();
static gid_t fp_gid = getgid();
static std::vector<gid_t> fp_gids{};

void initialize_fp_gids() {
    static bool fp_gids_initialised = false;
    if (! fp_gids_initialised) {
        int ngroups = getgroups(0, NULL);
        std::vector<gid_t> my_gids;
        my_gids.resize(ngroups);
        getgroups(ngroups, my_gids.data());
        fp_gids = std::move(my_gids);
        fp_gids_initialised = true;
    }
}

std::optional<std::filesystem::file_time_type> FileCtrl::fileLastWriteTime(filepath const & fp) noexcept
{
    try {
        return std::filesystem::last_write_time(fp);
    } catch (...) { return {}; }
}

std::optional<uint64_t> FileCtrl::fileSize(filepath const & fp) noexcept
{
    try {
        return std::filesystem::file_size(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::fileExists(filepath const & fp) noexcept
{
    try {
        return std::filesystem::exists(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::isFileReadable(filepath const & fp) noexcept
{
    try {
        if (! std::filesystem::exists(fp)) { return false; }
        auto s = std::filesystem::status(fp);
        if (! std::filesystem::is_regular_file(s)) { return false; }
        struct stat finfo;
        if (stat(fp.c_str(), &finfo) != 0) { return false; }
        auto we_are_owner = [&finfo]() -> bool { return finfo.st_uid == fp_uid; };
        auto we_are_group = [&finfo]() -> bool { return finfo.st_gid == fp_gid; };
        // we could also test that we do not have write permissions on these directories, just-to-be-sure (tm)
        auto user_r_access = [&finfo]() -> bool { return finfo.st_mode & S_IRUSR; };
        auto group_r_access = [&finfo]() -> bool { return finfo.st_mode & S_IRGRP; };
        auto other_r_access = [&finfo]() -> bool { return finfo.st_mode & S_IROTH; };
        /* user == uid has read access */
        if (we_are_owner() && user_r_access()) { return true; }
        /* check if others have read access */
        if (other_r_access()) { return true; }
        /* our group has read access */
        if (we_are_group() && group_r_access()) { return true; }
        /* we are in a group which has read access */
        if (group_r_access()) {
            initialize_fp_gids();
            for (auto g : fp_gids) {
                if (g == finfo.st_gid) { return true; }
            }
        }
    } catch (...) {}
    return false;
}

bool FileCtrl::dirExists(filepath const & fp) noexcept
{
    try {
        auto s = std::filesystem::status(fp);
        if (std::filesystem::is_symlink(s) && fp.string().find("../") != std::string::npos) { return false; } // prevent loop
        if (! std::filesystem::is_directory(s)) { return false; }
        struct stat finfo;
        if (stat(fp.c_str(), &finfo) != 0) {
            return false;
        }
        auto we_are_owner = [&finfo]() -> bool { return finfo.st_uid == fp_uid; };
        auto we_are_group = [&finfo]() -> bool { return finfo.st_gid == fp_gid; };
        // we could also test that we do not have write permissions on these directories, just-to-be-sure (tm)
        auto user_rx_access = [&finfo]() -> bool { return finfo.st_mode & S_IRUSR && finfo.st_mode & S_IXUSR; };
        auto group_rx_access = [&finfo]() -> bool { return finfo.st_mode & S_IRGRP && finfo.st_mode & S_IXGRP; };
        auto other_rx_access = [&finfo]() -> bool { return finfo.st_mode & S_IROTH && finfo.st_mode & S_IXOTH; };
        /* user == uid has r,x access */
        if (we_are_owner() && user_rx_access()) { return true; }
        /* check if others have r,x access */
        if (other_rx_access()) { return true; }
        /* our group has read access */
        if (we_are_group() && group_rx_access()) { return true; }
        /* we are in a group which has r,x access */
        if (group_rx_access()) {
            initialize_fp_gids();
            for (auto g : fp_gids) {
                if (g == finfo.st_gid) { return true; }
            }
        }
    } catch (...) {}
    return false;
}

std::generator<filepath> FileCtrl::fileListRecursive(const filepath fp)
{
    std::filesystem::directory_iterator dirit{fp};
    while (dirit != std::filesystem::directory_iterator{}) {
        if (auto const & fp2 = *dirit++; dirExists(fp2)) {
            for (auto const & fp3 : fileListRecursive(fp2)) {
                co_yield fp3;
            }
        } else {
            co_yield fp2;
        }
    }
}

} // namespace