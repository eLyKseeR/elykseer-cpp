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

// Boost link dynamically

//#ifndef BOOST_ALL_DYN_LINK
//#define BOOST_ALL_DYN_LINK
//#endif

// Boost test main entry point

#define BOOST_TEST_MAIN

// Boost test module

#define BOOST_TEST_MODULE UnitTests

// Boost test includes

#include "boost/test/unit_test.hpp"
