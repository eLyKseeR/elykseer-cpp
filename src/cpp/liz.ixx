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

export module lxr_liz;


export namespace lxr {


class Liz
{
    public:
        static const bool verify() noexcept;
        static const int daysLeft() noexcept;
        static const std::string license() noexcept;
        static const std::string copyright() noexcept;
        static const std::string version() noexcept;
        static const std::string name() noexcept;
    protected:
        Liz() {}
    private:
        Liz(Liz const &) = delete;
        Liz & operator=(Liz const &) = delete;
};

} // namespace