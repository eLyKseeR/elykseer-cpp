```cpp

/*
    eLyKseeR or LXR - cryptographic data archiving software
    https://github.com/eLyKseeR/elykseer-cpp
    Copyright (C) 2023 Alexander Diemand

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
```

`#pragma once`

`#include <optional>`

`#include <string>`

`#include "lxr/key256.hpp"`


namespace [lxr](namespace.list) {

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


# class CoqFilesupport

{

>public:

>typedef std::string filename;


## struct FileInformation

{

>filename fname;

>uint64_t fsize;

>std::string fowner;

>uint32_t fpermissions;

>std::string fmodified;

>std::string fchecksum;

};


>static std::optional&lt;FileInformation&gt; [get_file_information](coqfilesupport_functions.cpp.md)(const filename &) noexcept;

>private:

>CoqFilesupport() {}

>CoqFilesupport(const CoqFilesupport &) = delete;

>CoqFilesupport & operator=(const CoqFilesupport &) = delete;

};

```cpp
} // namespace
```
