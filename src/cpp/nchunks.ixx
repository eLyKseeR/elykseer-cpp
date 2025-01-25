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


export module lxr_nchunks;


export namespace lxr {

class Nchunks
{
    public:
        static constexpr int min_n { 16 };
        static constexpr int max_n { 256 };
        explicit Nchunks(int n);
        Nchunks(Nchunks const &) = default;
        Nchunks & operator=(Nchunks const &) = default;
        ~Nchunks() = default;
        int nchunks() const;
    private:
        Nchunks();
        int _n {min_n};
};

} // namespace