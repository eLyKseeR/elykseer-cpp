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

#include <string>
#include <utility>
#include <filesystem>
typedef std::filesystem::path filepath;

import lxr_filectrl;


export module lxr_fsutils;


export namespace lxr {

class FsUtils
{
    public:
        static std::string sep() noexcept;
        static const filepath cleanfp(filepath const &) noexcept;
        static std::pair<const std::string, const std::string> osusrgrp(filepath const &);
        static const std::string fstem() noexcept;
        static const filepath tempdir() noexcept;
        static const filepath tempfile() noexcept;
    protected:
        FsUtils() {}
    private:
        FsUtils(FsUtils const &) = delete;
        FsUtils & operator=(FsUtils const &) = delete;
};


const filepath operator/(std::string const &a, std::string const &b) noexcept;
const filepath operator/(filepath const &a, std::string const &b) noexcept;

} // namespace