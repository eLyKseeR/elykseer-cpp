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
#include <filesystem>
#include <fstream>
#include <sstream>

#include "boost/test/unit_test.hpp"

import lxr_coqassembly;
import lxr_coqconfiguration;
import lxr_coqbuffer;
import lxr_coqenvironment;
import lxr_key128;
import lxr_coqstore;


BOOST_AUTO_TEST_SUITE( utCoqEnvironment )

// Test case: create a _CoqEnvironmentWritable_

// an empty assembly is not encrypted and extracted.

BOOST_AUTO_TEST_CASE( instantiate_CoqEnvironmentWritable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqEnvironmentWritable _env(_config);
    auto keys = _env.finalise_assembly();

    BOOST_CHECK(! keys);
}

// Test case: create a _CoqEnvironmentReadable_

BOOST_AUTO_TEST_CASE( instantiate_CoqEnvironmentReadable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqEnvironmentReadable _env(_config);
    // auto keys = _env.finalise_assembly();

    // BOOST_CHECK(! keys);
}

// Test case: roundtrip of backup, finish, encrypt, extract, recall, decrypt, restore

BOOST_AUTO_TEST_CASE( backup_restore_roundtrip )
{
    lxr::CoqConfiguration _config;
    {
        _config.my_id = "hello world.";
        _config.nchunks(16);
        std::filesystem::path chunk_path = std::filesystem::temp_directory_path();
        lxr::Key128 _r;
        chunk_path /= _r.toHex();
        if (! std::filesystem::exists(chunk_path)) {
            std::filesystem::create_directory(chunk_path);
        }
        _config.path_chunks = (chunk_path /= "lxr").string();
        _config.path_db = (chunk_path /= "meta").string();
    }

    std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore()};
    lxr::CoqEnvironmentWritable _wenv(_config);
    _wenv.recreate_assembly();

    // backup
    const std::string msg{"good morning and hello world."};
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    auto backup_results = _wenv.backup("/data/test/message.txt", 0, buffer, buffer.len());
    auto fblocks = backup_results.first;
    BOOST_CHECK_EQUAL(1, fblocks.size());
    for (auto const & fb : fblocks) {
        _fblockstore->add(fb.first, fb.second);
    }

    // finish
    auto _keys = _wenv.finalise_assembly();
    BOOST_CHECK(_keys);

    // recall
    lxr::CoqEnvironmentReadable _renv(_config);
    BOOST_REQUIRE(_renv.restore_assembly(_keys.value().first, _keys.value().second));

    // test restored bytes
    lxr::CoqAssembly::BlockInformation bi = _fblockstore->at(0)->second->front();
    BOOST_CHECK_EQUAL(bi.blocksize, msg.length());
    std::shared_ptr<lxr::CoqBufferPlain> restored_block = _renv._assembly->restore(bi);
    BOOST_REQUIRE(restored_block);
    char restored_buffer[bi.blocksize];
    restored_block->to_buffer(bi.blocksize, restored_buffer);
    BOOST_CHECK_EQUAL(std::string(restored_buffer, bi.blocksize), msg);
}

static lxr::CoqConfiguration temp_config()
{
    lxr::CoqConfiguration _config;
    _config.my_id = "hello world.";
    _config.nchunks(16);
    std::filesystem::path chunk_path = std::filesystem::temp_directory_path();
    lxr::Key128 _r;
    chunk_path /= _r.toHex();
    std::filesystem::create_directories(chunk_path / "lxr");
    _config.path_chunks = (chunk_path / "lxr").string();
    _config.path_db = (chunk_path / "meta").string();
    return _config;
}

// Test case: a block that does not fit goes as a whole into a new assembly

BOOST_AUTO_TEST_CASE( block_not_split )
{
    auto _config = temp_config();
    lxr::CoqEnvironmentWritable _wenv(_config);
    _wenv.recreate_assembly();
    const std::string msg(32768, 'a');
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    // capacity: 16 * 262144 - 16 * 16 bytes = 127 blocks of 32768 and 32512 bytes
    const auto aid1 = _wenv._assembly->aid();
    for (int i = 0; i < 127; i++) {
        auto res = _wenv.backup("file", i * 32768, buffer, buffer.len());
        BOOST_REQUIRE_EQUAL(1, res.first.size());
        BOOST_CHECK_EQUAL(0, res.second.size());
    }
    BOOST_CHECK_EQUAL(32512, _wenv._assembly->afree());
    auto res = _wenv.backup("file", 127 * 32768, buffer, buffer.len());
    BOOST_REQUIRE_EQUAL(1, res.first.size());
    BOOST_REQUIRE_EQUAL(1, res.second.size());
    BOOST_CHECK_EQUAL(aid1, res.second.front().first);
    auto const & bi = res.first.front().second;
    BOOST_CHECK_EQUAL(32768, bi.blocksize);
    BOOST_CHECK_EQUAL(127 * 32768, bi.filepos);
    BOOST_CHECK_NE(aid1, bi.blockaid);
    BOOST_CHECK_EQUAL(0, bi.blockapos);
}

// Test case: no key is returned for an assembly that could not be extracted

BOOST_AUTO_TEST_CASE( finalise_fails_without_chunks )
{
    auto _config = temp_config();
    lxr::CoqEnvironmentWritable _wenv(_config);
    _wenv.recreate_assembly();
    const std::string msg{"cannot be written"};
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    _wenv.backup("file", 0, buffer, buffer.len());
    // a regular file in place of the chunk directory: no chunk can be written
    // (unlike missing write permissions, this also holds when running as root)
    std::filesystem::remove(_config.path_chunks);
    std::ofstream(_config.path_chunks) << "not a directory";
    auto _keys = _wenv.finalise_assembly();
    BOOST_CHECK(! _keys);
    BOOST_CHECK_EQUAL(1, _wenv.n_failed);
}

// Test case: keys of other archive formats are refused

BOOST_AUTO_TEST_CASE( restore_refuses_other_format )
{
    auto _config = temp_config();
    lxr::CoqEnvironmentWritable _wenv(_config);
    _wenv.recreate_assembly();
    const std::string msg{"format version"};
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    _wenv.backup("file", 0, buffer, buffer.len());
    auto _keys = _wenv.finalise_assembly();
    BOOST_REQUIRE(_keys);
    auto ki = _keys.value().second;
    BOOST_CHECK_EQUAL(ki._version, "0.10.1");
    {
        lxr::CoqEnvironmentReadable _renv(_config);
        BOOST_CHECK(_renv.restore_assembly(_keys.value().first, ki));
    }
    for (const std::string v : {"", "0.9.4", "0.11.0"}) {
        ki._version = v;
        lxr::CoqEnvironmentReadable _renv(_config);
        BOOST_CHECK(! _renv.restore_assembly(_keys.value().first, ki));
    }
}

BOOST_AUTO_TEST_SUITE_END()
