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
typedef std::filesystem::path filepath;

#include <cstdint>
#include <optional>
#include <vector>

export module lxr_filectrl;


export namespace lxr {

class FileCtrl
{
    public:
        static std::optional<std::filesystem::file_time_type> fileLastWriteTime(filepath const &) noexcept;
        static std::optional<uint64_t> fileSize(filepath const &) noexcept;
        static bool fileExists(filepath const &) noexcept;
        static bool isFileReadable(filepath const &) noexcept;
        static bool dirExists(filepath const &) noexcept;
        static std::vector<filepath> fileListRecursive(filepath const &);
    protected:
        FileCtrl() {}
    private:
        FileCtrl(FileCtrl const &) = delete;
        FileCtrl & operator=(FileCtrl const &) = delete;
};

} // namespace