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

#pragma once

#include <algorithm>
#include <string>

```

namespace [lxr](namespace.list) {

/*

```coq
Module Export Configuration.

Record configuration : Type :=
    mkconfiguration
        { config_nchunks : Nchunks.t
        ; path_chunks : string
        ; path_db : string
        ; my_id : N
        }.

End Configuration.
```

*/

# struct CoqConfiguration

{
    public:

    inline void nchunks(const int _nch) { _nchunks = std::min(256, std::max(16,_nch)); }

    inline uint16_t nchunks() const { return _nchunks; }

    std::string path_chunks;

    std::string path_db;

    std::string my_id;

    CoqConfiguration & operator=(CoqConfiguration const &) = default;

    private:

    uint16_t _nchunks{16};

};


```cpp
} // namespace
```
