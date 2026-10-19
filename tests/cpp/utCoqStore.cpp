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
#include <sstream>

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
    _store.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", "fhash42", 3728, "user", 0644, "2024-12-31T14:21:42", "checksum42"}));
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
    _store.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", "fhash42", 3728, "user", 0644, "2024-12-31T14:21:42", "checksum42"}));
    BOOST_CHECK_EQUAL(_store.size(), 1);
    BOOST_CHECK(_store.contains("key1"));
    auto fi = _store.find("key1");
    BOOST_CHECK(fi);
    BOOST_CHECK_EQUAL(fi->fsize, 3728);
}

// Test case: Store.add: the newest file information wins, keys are never replaced

BOOST_AUTO_TEST_CASE( add_replaces_or_refuses )
{
    lxr::CoqFInfoStore _fistore;
    _fistore.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", "key1", 1, "user", 0644, "2024-12-31T14:21:42", "checksum1"}));
    BOOST_CHECK(_fistore.add("key1", lxr::CoqFilesupport::FileInformation({"test.txt", "key1", 2, "user", 0644, "2025-01-01T10:00:00", "checksum2"})));
    BOOST_CHECK_EQUAL(_fistore.find("key1")->fchecksum, "checksum2");

    lxr::CoqKeyStore _kstore;
    lxr::CoqAssembly::KeyInformation k1{}, k2{};
    BOOST_CHECK(_kstore.add("aid1", k1));
    BOOST_CHECK(! _kstore.add("aid1", k2));
    BOOST_CHECK_EQUAL(_kstore.find("aid1")->_pkey.toHex(), k1._pkey.toHex());
}

// Test case: keys document (schema/keys.xsd of elykseer-ml)

BOOST_AUTO_TEST_CASE( keys_xml_roundtrip )
{
    lxr::CoqKeyStore _store;
    lxr::CoqAssembly::KeyInformation ki{};
    ki._localnchunks = 42;
    _store.add("aid1", ki);
    std::stringstream ss;
    _store.outStream(ss);
    const std::string xml = ss.str();
    BOOST_CHECK(xml.find("<keys>") != std::string::npos);
    BOOST_CHECK(xml.find("<key nchunks=\"42\" version=\"0.10.1\" aid=\"aid1\"><secret>" + ki._pkey.toHex() + "</secret><iv>" + ki._ivec.toHex() + "</iv></key>") != std::string::npos);

    lxr::CoqKeyStore _store2;
    _store2.inStream(ss);
    BOOST_REQUIRE_EQUAL(_store2.size(), 1);
    auto ki2 = _store2.find("aid1");
    BOOST_REQUIRE(ki2);
    BOOST_CHECK_EQUAL(ki2->_pkey.toHex(), ki._pkey.toHex());
    BOOST_CHECK_EQUAL(ki2->_ivec.toHex(), ki._ivec.toHex());
    BOOST_CHECK_EQUAL(ki2->_localnchunks.u(), 42);
    BOOST_CHECK_EQUAL(ki2->_version, "0.10.1");
}

// Test case: an export of lxr_relkeys (elykseer-ml)

BOOST_AUTO_TEST_CASE( keys_xml_from_elykseer_ml )
{
    std::stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<keys>\n"
       << "<key nchunks=\"16\" version=\"0.10.1\" aid=\"7d1f\"><secret>000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f</secret><iv>f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff</iv></key>\n"
       << "</keys>\n";
    lxr::CoqKeyStore _store;
    _store.inStream(ss);
    auto ki = _store.find("7d1f");
    BOOST_REQUIRE(ki);
    BOOST_CHECK_EQUAL(ki->_pkey.toHex(), "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
    BOOST_CHECK_EQUAL(ki->_ivec.toHex(), "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff");
}

// Test case: files document (schema/fileinformation.xsd of elykseer-ml)

BOOST_AUTO_TEST_CASE( files_xml_roundtrip )
{
    std::shared_ptr<lxr::CoqFInfoStore> _fistore{new lxr::CoqFInfoStore()};
    std::shared_ptr<lxr::CoqFBlockStore> _fbstore{new lxr::CoqFBlockStore()};
    const std::string fhash{"0123abcd"};
    _fistore->add(fhash, lxr::CoqFilesupport::FileInformation({"dir/a & <b>.txt", fhash, 40000, "501", 644, "2025-01-01T10:00:00", "checksum42"}));
    auto chk1 = lxr::Sha3_256::hash(std::string("block 1"));
    auto chk2 = lxr::Sha3_256::hash(std::string("block 2"));
    _fbstore->add(fhash, lxr::CoqAssembly::BlockInformation({1, chk1, 32768, 0, "aid001", 0}));
    _fbstore->add(fhash, lxr::CoqAssembly::BlockInformation({2, chk2, 7232, 32768, "aid002", 128}));
    // a file without blocks in this backup
    _fistore->add("ffff", lxr::CoqFilesupport::FileInformation({"unchanged", "ffff", 1, "501", 600, "2025-01-01T10:00:00", "checksum1"}));

    std::stringstream ss;
    lxr::CoqFilesStream(_fistore, _fbstore).outStream(ss);
    const std::string xml = ss.str();
    BOOST_CHECK(xml.find("<files>") != std::string::npos);
    BOOST_CHECK(xml.find("<fileinformation version=\"0.10.1\" fhash=\"0123abcd\">") != std::string::npos);
    BOOST_CHECK(xml.find("<name>dir/a &amp; &lt;b&gt;.txt</name>") != std::string::npos);
    BOOST_CHECK(xml.find("<fileblock blockid=\"2\" blocksize=\"7232\" filepos=\"32768\" apos=\"128\"><aid>aid002</aid><checksum>" + chk2.toHex() + "</checksum></fileblock>") != std::string::npos);

    // each store reads its part
    std::stringstream ss1(xml), ss2(xml);
    lxr::CoqFInfoStore _fistore2;
    _fistore2.inStream(ss1);
    BOOST_CHECK_EQUAL(_fistore2.size(), 2);
    auto fi = _fistore2.find(fhash);
    BOOST_REQUIRE(fi);
    BOOST_CHECK_EQUAL(fi->fname, "dir/a & <b>.txt");
    BOOST_CHECK_EQUAL(fi->fhash, fhash);
    BOOST_CHECK_EQUAL(fi->fsize, 40000);
    BOOST_CHECK_EQUAL(fi->fpermissions, 644);
    BOOST_CHECK_EQUAL(fi->fchecksum, "checksum42");

    lxr::CoqFBlockStore _fbstore2;
    _fbstore2.inStream(ss2);
    BOOST_CHECK_EQUAL(_fbstore2.size(), 1);
    auto bis = _fbstore2.find(fhash);
    BOOST_REQUIRE(bis);
    BOOST_REQUIRE_EQUAL(bis.value()->size(), 2);
    auto const & b2 = bis.value()->at(1);
    BOOST_CHECK_EQUAL(b2.blockid, 2);
    BOOST_CHECK_EQUAL(b2.blocksize, 7232);
    BOOST_CHECK_EQUAL(b2.filepos, 32768);
    BOOST_CHECK_EQUAL(b2.blockapos, 128);
    BOOST_CHECK_EQUAL(b2.blockaid, "aid002");
    BOOST_CHECK_EQUAL(b2.bchecksum.toHex(), chk2.toHex());
}

// Test case: meta data of elykseer-cpp < 5 is not loaded

BOOST_AUTO_TEST_CASE( legacy_xml_ignored )
{
    std::stringstream ss;
    ss << "<?xml version=\"1.0\"?>\n<DbKey xmlns=\"http://spec.elykseer.com/lxr/v1.0\">\n"
       << "  <Key n=\"16\" iv=\"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff\" pk=\"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f\">7d1f</Key>\n"
       << "</DbKey>\n";
    lxr::CoqKeyStore _store;
    _store.inStream(ss);
    BOOST_CHECK_EQUAL(_store.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
