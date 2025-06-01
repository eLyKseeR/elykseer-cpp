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

import lxr_fsutils;
import lxr_md5;


BOOST_AUTO_TEST_SUITE( utFsUtils )

// Test case: get file stem
BOOST_AUTO_TEST_CASE( file_stem )
{
	std::string fstem = lxr::FsUtils::fstem();
	//std::clog << lxr::FsUtils::fstem() << std::endl;
	BOOST_CHECK_EQUAL(fstem.substr(0,4), "lxr_");
}

// Test case: get file stem
BOOST_AUTO_TEST_CASE( get_user_grp_owner )
{
	auto _usrgrp = lxr::FsUtils::osusrgrp("/bin/sh");
	//std::clog << _usrgrp.first << " " << _usrgrp.second << std::endl;
	BOOST_CHECK_EQUAL(_usrgrp.first, "root");
#ifdef __APPLE__
	BOOST_CHECK_EQUAL(_usrgrp.second, "wheel");
#endif
#ifdef __linux__
	BOOST_WARN_EQUAL(_usrgrp.second, "root");
#endif
}

BOOST_AUTO_TEST_SUITE_END()