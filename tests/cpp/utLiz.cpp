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

import lxr_liz;


BOOST_AUTO_TEST_SUITE( utLiz )
// Test case: show copyright message
BOOST_AUTO_TEST_CASE( copyright_message )
{
	std::string msg = lxr::Liz::copyright();
	std::string veritas = "Copyright";
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK_EQUAL(msg.substr(0,9), veritas);
}

// Test case: show version message
BOOST_AUTO_TEST_CASE( version_message )
{
	std::string msg = lxr::Liz::version();
	std::string veritas = "Version";
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK_EQUAL(msg.substr(0,7), veritas);
}

// Test case: show license message
BOOST_AUTO_TEST_CASE( license_message )
{
	std::string msg = lxr::Liz::license();
	std::string veritas = "                    GNU GENERAL PUBLIC LICENSE";
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK_EQUAL(msg.substr(0,46), veritas);
}

// Test case: show name message
BOOST_AUTO_TEST_CASE( name_message )
{
	std::string msg = lxr::Liz::name();
	std::string veritas = "LXR";
	//std::clog << std::endl << msg << std::endl;
	BOOST_CHECK_EQUAL(msg.substr(0,3), veritas);
}

BOOST_AUTO_TEST_SUITE_END()