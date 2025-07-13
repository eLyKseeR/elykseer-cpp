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

#include <memory>

#include "boost/test/unit_test.hpp"

import lxr_coqassembly;
import lxr_coqfilesupport;
import lxr_coqstore;
import lxr_fsutils;
import lxr_key128;
import lxr_key256;
import lxr_random;
import lxr_sha3;
import lxr_os;


BOOST_AUTO_TEST_SUITE( utCoqStore )

// Test case: create a _CoqKeyStore_

BOOST_AUTO_TEST_CASE( instantiate_CoqKeyStore )
{
    lxr::CoqKeyStore _store;
    BOOST_CHECK_EQUAL(_store.size(), 0);
}

// Test case: create a _CoqFBlockStore_

BOOST_AUTO_TEST_CASE( instantiate_CoqFBlockStore )
{
    lxr::CoqFBlockStore _store;
    BOOST_CHECK_EQUAL(_store.size(), 0);
}

// Test case: create a _CoqFInfoStore_

BOOST_AUTO_TEST_CASE( instantiate_CoqFInfoStore )
{
    lxr::CoqFInfoStore _store;
    BOOST_CHECK_EQUAL(_store.size(), 0);
}

// Test case: add entry

BOOST_AUTO_TEST_CASE( add_to_CoqKeyStore )
{
    lxr::CoqKeyStore _store;
    _store.add("key1", lxr::CoqAssembly::KeyInformation());
    BOOST_CHECK_EQUAL(_store.size(), 1);
}

// Test case: add entry

BOOST_AUTO_TEST_CASE( add_to_CoqFBlockStore )
{
    lxr::CoqFBlockStore _store;
    _store.add("key1", lxr::CoqAssembly::BlockInformation({42, lxr::Key256(), 37221, 949193, "aid001", 16}));
    _store.add("key1", lxr::CoqAssembly::BlockInformation({43, lxr::Key256(), 37221, 949193, "aid001", 16}));
    BOOST_CHECK_EQUAL(_store.size(), 1);
}

// Test case: add entry

BOOST_AUTO_TEST_CASE( add_to_CoqFInfoStore )
{
    lxr::CoqFInfoStore _store;
    _store.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", 3728, "user", 0644, "2024-12-31T14:21:42", "checksum42"}));
    BOOST_CHECK_EQUAL(_store.size(), 1);
}

// Test case: find entry

BOOST_AUTO_TEST_CASE( find_in_CoqKeyStore )
{
    lxr::CoqKeyStore _store;
    lxr::CoqAssembly::KeyInformation _ki{};
    _store.add("key1", _ki);
    BOOST_CHECK_EQUAL(_store.size(), 1);
    auto ki = _store.find("key1");
    BOOST_CHECK(ki);
    BOOST_CHECK_EQUAL(ki->_localnchunks.i(), 16);
}

// Test case: find entry

BOOST_AUTO_TEST_CASE( find_in_CoqFBlockStore )
{
    lxr::CoqFBlockStore _store;
    _store.add("key1", lxr::CoqAssembly::BlockInformation({42, lxr::Key256(), 37221, 949193, "aid001", 16}));
    _store.add("key1", lxr::CoqAssembly::BlockInformation({43, lxr::Key256(), 283, 949193+37221, "aid001", 16}));
    BOOST_CHECK_EQUAL(_store.size(), 1);
    auto vp = _store.find("key1");
    BOOST_CHECK(vp);
    std::shared_ptr<std::vector<lxr::CoqAssembly::BlockInformation>> vbi = vp.value();
    BOOST_CHECK_EQUAL(vbi->size(), 2);
    BOOST_CHECK_EQUAL(vbi->at(0).blocksize, 37221);
    BOOST_CHECK_EQUAL(vbi->at(1).blocksize, 283);
}

// Test case: find entry

BOOST_AUTO_TEST_CASE( find_in_CoqFInfoStore )
{
    lxr::CoqFInfoStore _store;
    _store.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", 3728, "user", 0644, "2024-12-31T14:21:42", "checksum42"}));
    BOOST_CHECK_EQUAL(_store.size(), 1);
    BOOST_CHECK(_store.contains("key1"));
    auto fi = _store.find("key1");
    BOOST_CHECK(fi);
    BOOST_CHECK_EQUAL(fi->fsize, 3728);
}

BOOST_AUTO_TEST_SUITE_END()