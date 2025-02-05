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

#include <algorithm>


module lxr_nchunks;


namespace lxr {

Nchunks::Nchunks(int n) noexcept
{
    _n = std::min(ChunkRange::max_n, std::max(ChunkRange::min_n, n));
}

int Nchunks::i() const noexcept
{
    return _n;
}

unsigned int Nchunks::u() const noexcept
{
    return _n;
}

std::generator<int> Nchunks::sequence() const {
    int num = 1;
    while (num <= _n) {
        co_yield num++;
    }
}

} // namespace