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

#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include <iostream>

#include "boost/test/unit_test.hpp"

import lxr_os;


BOOST_AUTO_TEST_SUITE( utOS )
// Test case: show hostname
BOOST_AUTO_TEST_CASE( get_hostname )
{
	std::string msg = lxr::OS::hostname();
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK(! msg.empty());
}

// Test case: show username
BOOST_AUTO_TEST_CASE( get_username )
{
	std::string msg = lxr::OS::username();
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK(! msg.empty());
	BOOST_CHECK_NE(msg, "unknown");
}

// Test case: show timestamp
BOOST_AUTO_TEST_CASE( get_timestamp )
{
	std::string msg = lxr::OS::timestamp();
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK(! msg.empty());
}

BOOST_AUTO_TEST_SUITE_END()