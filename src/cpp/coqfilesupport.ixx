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

#include <optional>
#include <string>

import lxr_key256;


export module lxr_coqfilesupport;


export namespace lxr {

/*

```coq
Definition filename := string.

Record fileinformation : Type := mkfileinformation
     { fname : filename
     ; fsize : N
     ; fowner : string
     ; fpermissions : N
     ; fmodified : string
     ; fchecksum : string
     }.

Axiom get_file_information : filename -> fileinformation.

```
*/


class CoqFilesupport
{
    public:
        typedef std::string filename;

        struct FileInformation
        {
            filename fname;
            uint64_t fsize;
            std::string fowner;
            uint32_t fpermissions;
            std::string fmodified;
            std::string fchecksum;
        };

        static std::optional<struct FileInformation> get_file_information(const filename &) noexcept;
    private:
        CoqFilesupport() {}
        CoqFilesupport(const CoqFilesupport &) = delete;
        CoqFilesupport & operator=(const CoqFilesupport &) = delete;
};

} // namespace