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
#include <filesystem>
#include <chrono>
#include <ctime>

export module lxr_os;

export namespace lxr {

class OS
{
    public:
        static const std::string hostname() noexcept;
        static const std::string username() noexcept;
        static const std::string timestamp() noexcept;
        static const std::string time2string(time_t) noexcept;
        static const std::string time2string(std::filesystem::file_time_type) noexcept;
    protected:
        OS() {}
    private:
        OS(OS const &) = delete;
        OS & operator=(OS const &) = delete;
};

} /* namespace */

std::ostream& operator<<(std::ostream& os, std::chrono::microseconds const &us); 
