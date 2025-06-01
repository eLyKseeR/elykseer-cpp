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

#include <optional>

#include "boost/test/unit_test.hpp"

import lxr_filectrl;
import lxr_md5;


BOOST_AUTO_TEST_SUITE( utFileCtrl )

// Test case: get file size, date, last write time
BOOST_AUTO_TEST_CASE( get_file_information )
{
	const std::string fp = "/bin/sh";
	BOOST_CHECK(lxr::FileCtrl::fileExists(fp));
	BOOST_CHECK(lxr::FileCtrl::isFileReadable(fp));
	BOOST_CHECK(! lxr::FileCtrl::dirExists(fp));
	// BOOST_CHECK(lxr::FileCtrl::fileDate(fp) > "2000");
	BOOST_CHECK(lxr::FileCtrl::fileSize(fp) > 6000ULL);
}

// Test case: get recursive directory listing
BOOST_AUTO_TEST_CASE( get_directory_listing )
{
#ifdef _WIN32
    const std::string fp = "C:\\Windows\\System32";
#else
    const std::string fp =
        lxr::FileCtrl::dirExists("/run/current-system/sw/share/")?"/run/current-system/sw/share/":
        lxr::FileCtrl::dirExists("/usr/share")?"/usr/share":"/usr/bin";
#endif
    int counter{0};
    for (auto const & _fplisting : lxr::FileCtrl::fileListRecursive(fp)) { counter++; }
    BOOST_WARN( counter > 1 );
}

BOOST_AUTO_TEST_SUITE_END()