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
import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_key128;


BOOST_AUTO_TEST_SUITE( utCoqAssembly )

// Test case: cannot directly instantiate CoqAssembly

// because _CoqAssembly_ is an abstract class we cannot create an instance of it.
// expect this test to fail if the _#ifdef_ is commented out.

#ifdef CHECK_EXPECTED_COMPILER_ERRORS
BOOST_AUTO_TEST_CASE( cannot_instantiate_CoqAssembly_directly )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssembly _assembly(_config);

    BOOST_CHECK(_assembly.assemblysize(16) > 0);
}
#endif

// Test case: create a _CoqAssemblyPlainWritable_

BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyPlainWritable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyPlainWritable _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(16, _assembly.apos());
}

// Test case: create a _CoqAssemblyPlainFull_

BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyPlainFull )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyPlainFull _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(0, _assembly.apos());
}

// Test case: create a _CoqAssemblyEncrypted_

BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyEncrypted )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyEncrypted _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(0, _assembly.apos());
}

// Test case: roundtrip of backup, finish, encrypt, extract, recall, decrypt, restore

BOOST_AUTO_TEST_CASE( backup_restore_roundtrip )
{
    lxr::CoqConfiguration _config;
    _config.my_id = "hello world.";
    _config.nchunks(17);
    std::filesystem::path chunk_path = std::filesystem::temp_directory_path();
    lxr::Key128 _r;
    chunk_path /= _r.toHex();
    if (! std::filesystem::exists(chunk_path)) {
        std::filesystem::create_directory(chunk_path);
    }
    _config.path_chunks = (chunk_path /= "lxr").string();
    _config.path_db = (chunk_path /= "meta").string();
    
    lxr::CoqAssemblyPlainWritable _assembly(_config);
    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 17 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(16, _assembly.apos());

    // backup
    const std::string msg{"good morning and hello world\n."};
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    lxr::CoqAssembly::BlockInformation bi = _assembly.backup(buffer);
    BOOST_CHECK_EQUAL(16 + msg.length(), _assembly.apos());

    // finish
    std::shared_ptr<lxr::CoqAssemblyPlainFull> afinished = _assembly.finish();
    BOOST_CHECK(afinished);
    if (auto h1 = afinished->calc_checksum(); h1) {
        std::clog << "CoqAssemblyPlainFull checksum: " << h1.value().toHex() << std::endl;
    } else {
        std::clog << "CoqAssemblyPlainFull failed to compute checksum" << std::endl;
    }

    // encrypt
    lxr::CoqAssembly::KeyInformation ki(_config);
    std::clog << "ivec: " << ki._ivec.toHex() << std::endl;
    std::clog << "pkey: " << ki._pkey.toHex() << std::endl;
    std::shared_ptr<lxr::CoqAssemblyEncrypted> aencrypted = afinished->encrypt(ki);

    // extract
    uint32_t n_ex = aencrypted->extract();

    // recall
    std::shared_ptr<lxr::CoqAssemblyEncrypted> arecalled = lxr::CoqAssemblyEncrypted::recall(_config, aencrypted->aid());

    // decrypt
    std::shared_ptr<lxr::CoqAssemblyPlainFull> adecrypted = arecalled->decrypt(ki);

    // restore
    std::shared_ptr<lxr::CoqBufferPlain> restored_block = adecrypted->restore(bi);

    // test restored bytes
    BOOST_CHECK_EQUAL(bi.blocksize, msg.length());
    char restored_buffer[bi.blocksize];
    restored_block->to_buffer(bi.blocksize, restored_buffer);
    BOOST_CHECK_EQUAL(std::string(restored_buffer, bi.blocksize), msg);
}

BOOST_AUTO_TEST_SUITE_END()