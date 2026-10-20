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
#include <stdexcept>

#include "boost/test/unit_test.hpp"

import lxr_coqassembly;
import lxr_coqbuffer;
import lxr_coqconfiguration;
import lxr_key128;
import lxr_key256;
import lxr_sha3;


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
    BOOST_CHECK_EQUAL(0, _assembly.apos());
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
    // random header of min(n - 16, 128) bytes
    BOOST_CHECK_EQUAL(1, _assembly.apos());

    // backup
    const std::string msg{"good morning and hello world\n."};
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    lxr::CoqAssembly::BlockInformation bi = _assembly.backup(buffer);
    BOOST_CHECK_EQUAL(1 + msg.length(), _assembly.apos());
    BOOST_CHECK_EQUAL(1, bi.blockapos);

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

    BOOST_REQUIRE(aencrypted);

    // extract
    uint32_t n_ex = aencrypted->extract();
    BOOST_CHECK_EQUAL(n_ex, 17 * lxr::CoqAssembly::chunksize);

    // recall
    std::shared_ptr<lxr::CoqAssemblyEncrypted> arecalled = lxr::CoqAssemblyEncrypted::recall(_config, aencrypted->aid());
    BOOST_REQUIRE(arecalled);

    // decrypt
    std::shared_ptr<lxr::CoqAssemblyPlainFull> adecrypted = arecalled->decrypt(ki);
    BOOST_REQUIRE(adecrypted);

    // restore
    std::shared_ptr<lxr::CoqBufferPlain> restored_block = adecrypted->restore(bi);
    BOOST_REQUIRE(restored_block);

    // test restored bytes
    BOOST_CHECK_EQUAL(bi.blocksize, msg.length());
    char restored_buffer[bi.blocksize];
    restored_block->to_buffer(bi.blocksize, restored_buffer);
    BOOST_CHECK_EQUAL(std::string(restored_buffer, bi.blocksize), msg);
}

// Test case: layout constants of the archive format (doc/08_Format.md)

BOOST_AUTO_TEST_CASE( format_layout )
{
    BOOST_CHECK_EQUAL(0, lxr::CoqAssembly::header_len(16));
    BOOST_CHECK_EQUAL(1, lxr::CoqAssembly::header_len(17));
    BOOST_CHECK_EQUAL(128, lxr::CoqAssembly::header_len(144));
    BOOST_CHECK_EQUAL(128, lxr::CoqAssembly::header_len(256));

    // logical index i -> chunk (i mod n), offset (i div n)
    BOOST_CHECK_EQUAL(0, lxr::CoqAssembly::idx2apos(0, 16));
    BOOST_CHECK_EQUAL(262144, lxr::CoqAssembly::idx2apos(1, 16));
    BOOST_CHECK_EQUAL(15 * 262144, lxr::CoqAssembly::idx2apos(15, 16));
    BOOST_CHECK_EQUAL(1, lxr::CoqAssembly::idx2apos(16, 16));
    BOOST_CHECK_EQUAL(262145, lxr::CoqAssembly::idx2apos(17, 16));
    BOOST_CHECK_EQUAL(16 * 262144 - 1, lxr::CoqAssembly::idx2apos(16 * 262144 - 1, 16));

    // the last 16 rows are reserved for the GCM tag
    lxr::CoqConfiguration _config;
    _config.nchunks(17);
    lxr::CoqAssemblyPlainWritable _assembly(_config);
    BOOST_CHECK_EQUAL(_assembly.afree(), 17 * 262144 - 1 - 16 * 17);
}

// Test case: identifiers

BOOST_AUTO_TEST_CASE( format_identifiers )
{
    const std::string myid{"1234567890"};
    const std::string rnd{"00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff"};
    BOOST_CHECK_EQUAL(lxr::CoqAssembly::mk_aid(myid, rnd), lxr::Sha3_256::hash(myid + rnd).toHex());
    const std::string aid = lxr::CoqAssembly::mk_aid(myid, rnd);
    BOOST_CHECK_EQUAL(lxr::CoqAssembly::chunk_identifier(myid, aid, 3), lxr::Sha3_256::hash(myid + "3" + aid).toHex());
    BOOST_CHECK_EQUAL(lxr::CoqAssembly::chunk_identifier(myid, aid, 16), lxr::Sha3_256::hash(myid + "16" + aid).toHex());

    lxr::CoqConfiguration _config;
    _config.my_id = myid;
    lxr::CoqAssemblyPlainWritable a1(_config);
    lxr::CoqAssemblyPlainWritable a2(_config);
    BOOST_CHECK_EQUAL(64, a1.aid().length());
    BOOST_CHECK_NE(a1.aid(), a2.aid());
}

// Test case: AES-256-GCM authenticates key, nonce, aad and data

BOOST_AUTO_TEST_CASE( buffer_gcm )
{
    const int len = 4096 * 3 + 100;
    std::string msg(len, 'x');
    for (int i = 0; i < len; i++) { msg[i] = (char)(i * 7 + 3); }
    const lxr::Key128 iv;
    const lxr::Key256 key;
    const std::string aad{"the aid"};

    auto encrypted = [&]() {
        lxr::CoqBufferPlain b(len, msg.c_str());
        auto e = b.encrypt(iv, key, aad);
        BOOST_REQUIRE(e);
        return e;
    };

    // roundtrip
    {
        auto e = encrypted();
        auto d = e->decrypt(iv, key, aad);
        BOOST_REQUIRE(d);
        char out[len];
        d->to_buffer(len, out);
        // the last 16 bytes held the tag
        BOOST_CHECK_EQUAL(std::string(out, len - 16), msg.substr(0, len - 16));
    }
    // wrong aad
    BOOST_CHECK(! encrypted()->decrypt(iv, key, "other aid"));
    // wrong key
    BOOST_CHECK(! encrypted()->decrypt(iv, lxr::Key256(), aad));
    // wrong nonce
    BOOST_CHECK(! encrypted()->decrypt(lxr::Key128(), key, aad));
    // modified data and modified tag
    for (int pos : {0, 5000, len - 17, len - 1}) {
        auto e = encrypted();
        e->at(pos, e->at(pos) ^ 0x01);
        BOOST_CHECK(! e->decrypt(iv, key, aad));
    }
}

static lxr::CoqConfiguration temp_config(int nchunks)
{
    lxr::CoqConfiguration _config;
    _config.my_id = "hello world.";
    _config.nchunks(nchunks);
    std::filesystem::path chunk_path = std::filesystem::temp_directory_path();
    lxr::Key128 _r;
    chunk_path /= _r.toHex();
    std::filesystem::create_directories(chunk_path);
    _config.path_chunks = (chunk_path / "lxr").string();
    _config.path_db = (chunk_path / "meta").string();
    return _config;
}

// writes an assembly with one block to chunks and returns its key and block information
static std::pair<lxr::CoqAssembly::KeyInformation, lxr::CoqAssembly::BlockInformation> extracted_assembly(const lxr::CoqConfiguration & _config, const std::string & msg)
{
    lxr::CoqAssemblyPlainWritable _assembly(_config);
    lxr::CoqBufferPlain buffer(msg.length(), msg.c_str());
    auto bi = _assembly.backup(buffer);
    lxr::CoqAssembly::KeyInformation ki(_config);
    auto aencrypted = _assembly.finish()->encrypt(ki);
    BOOST_REQUIRE(aencrypted);
    BOOST_REQUIRE_EQUAL(aencrypted->extract(), _config.nchunks().u() * lxr::CoqAssembly::chunksize);
    return {ki, bi};
}

static std::filesystem::path chunk_file(const lxr::CoqConfiguration & _config, const lxr::CoqAssembly::aid_t & aid, int cid)
{
    const auto cident = lxr::CoqAssembly::chunk_identifier(_config.my_id, aid, cid);
    return std::filesystem::path(_config.path_chunks) / cident.substr(62, 2) / cident.substr(60, 2) / (cident + ".lxr");
}

// Test case: buffer access out of range throws instead of touching the byte past the end

BOOST_AUTO_TEST_CASE( buffer_at_bounds )
{
    lxr::CoqBufferPlain b(16);
    b.at(15, 'x');
    BOOST_CHECK_EQUAL(b.at(15), 'x');
    BOOST_CHECK_THROW(b.at(16), std::out_of_range);
    BOOST_CHECK_THROW(b.at(16, 'y'), std::out_of_range);
    BOOST_CHECK_THROW(b.at(0xffffffff), std::out_of_range);
    lxr::CoqBufferPlain empty(0);
    BOOST_CHECK_THROW(empty.at(0), std::out_of_range);
}

// Test case: a modified chunk fails decryption

BOOST_AUTO_TEST_CASE( tampered_chunk_fails )
{
    auto _config = temp_config(16);
    auto [ki, bi] = extracted_assembly(_config, "some data to be modified");
    const auto fp = chunk_file(_config, bi.blockaid, 7);
    BOOST_REQUIRE(std::filesystem::exists(fp));
    {
        std::fstream f(fp, std::ios::in | std::ios::out | std::ios::binary);
        f.seekg(1000); char c; f.get(c);
        f.seekp(1000); f.put(c ^ 0x10);
    }
    auto arecalled = lxr::CoqAssemblyEncrypted::recall(_config, bi.blockaid);
    BOOST_REQUIRE(arecalled);
    BOOST_CHECK(! arecalled->decrypt(ki));
}

// Test case: a missing or short chunk fails recall

BOOST_AUTO_TEST_CASE( missing_chunk_fails )
{
    auto _config = temp_config(16);
    auto [ki, bi] = extracted_assembly(_config, "some data");
    BOOST_CHECK(lxr::CoqAssemblyEncrypted::recall(_config, bi.blockaid));
    std::filesystem::resize_file(chunk_file(_config, bi.blockaid, 16), 1000);
    BOOST_CHECK(! lxr::CoqAssemblyEncrypted::recall(_config, bi.blockaid));
    std::filesystem::remove(chunk_file(_config, bi.blockaid, 3));
    BOOST_CHECK(! lxr::CoqAssemblyEncrypted::recall(_config, bi.blockaid));
}

// Test case: a block whose checksum differs is not restored

BOOST_AUTO_TEST_CASE( restore_checks_block_checksum )
{
    auto _config = temp_config(16);
    const std::string msg{"verify this block"};
    auto [ki, bi] = extracted_assembly(_config, msg);
    auto adecrypted = lxr::CoqAssemblyEncrypted::recall(_config, bi.blockaid)->decrypt(ki);
    BOOST_REQUIRE(adecrypted);
    BOOST_CHECK(adecrypted->restore(bi));
    auto bi2 = bi;
    bi2.bchecksum = lxr::Sha3_256::hash(std::string("something else"));
    BOOST_CHECK(! adecrypted->restore(bi2));
    auto bi3 = bi;
    bi3.blockapos += 1;
    BOOST_CHECK(! adecrypted->restore(bi3));
    auto bi4 = bi;
    bi4.blockapos = 16 * lxr::CoqAssembly::chunksize - 4;
    BOOST_CHECK(! adecrypted->restore(bi4));
}

BOOST_AUTO_TEST_SUITE_END()
