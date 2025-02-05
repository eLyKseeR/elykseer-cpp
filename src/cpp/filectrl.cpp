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

module lxr_filectrl;


namespace lxr {

std::optional<std::filesystem::file_time_type> FileCtrl::fileLastWriteTime(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::last_write_time(fp);
    } catch (...) { return {}; }
}

std::optional<uint64_t> FileCtrl::fileSize(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::file_size(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::fileExists(std::filesystem::path const & fp) noexcept
{
    try {
        return std::filesystem::exists(fp);
    } catch (...) { return {}; }
}

bool FileCtrl::isFileReadable(std::filesystem::path const & fp) noexcept
{
    try {
        auto s = std::filesystem::status(fp);
        return std::filesystem::is_regular_file(s);
    } catch (...) {}
    return false;
}

bool FileCtrl::dirExists(std::filesystem::path const & fp) noexcept
{
    try {
        auto s = std::filesystem::status(fp);
        return std::filesystem::is_directory(s);
    } catch (...) {}
    return false;
}

std::generator<std::filesystem::path> FileCtrl::fileListRecursive(std::filesystem::path const & fp)
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