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
#include <cstdint>
#endif

#include "boost/test/unit_test.hpp"

#include <iostream>
// #include <fstream>
#include <filesystem>
// #include <sstream>
#include <variant>

import lxr_coqassembly;
import lxr_coqassemblycache;
import lxr_coqconfiguration;
import lxr_coqbuffer;
import lxr_coqstore;
import lxr_key256;
import lxr_key128;
import lxr_sha3;


BOOST_AUTO_TEST_SUITE( utCoqAssemblyCache )

// Test case: empty metrics on new _CoqAssemblyCache_

BOOST_AUTO_TEST_CASE( metrics_empty_for_new_CoqAssemblyCache )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyCache _qac(_config, 3, 12);

    auto metrics = _qac.metrics();
    std::map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics{};
    map_metrics.insert(metrics.begin(), metrics.end());

    for (auto const &e : map_metrics) {
        std::clog << "  " << e.first << ": ";
        if (std::holds_alternative<std::string>(e.second))
            std::clog << std::get<std::string>(e.second) << std::endl;
        if (std::holds_alternative<uint32_t>(e.second))
            std::clog << std::get<uint32_t>(e.second) << std::endl;
        if (std::holds_alternative<uint64_t>(e.second))
            std::clog << std::get<uint64_t>(e.second) << std::endl;
    }

    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics["n_read_environments"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics["n_write_environments"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics["len_read_queue"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics["len_write_queue"]));
}

// Test case: backup some data

BOOST_AUTO_TEST_CASE( backup_restore_roundtrip )
{
    lxr::CoqConfiguration _config;
    {
        _config.my_id = "assembly cache test";
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

    std::shared_ptr<lxr::CoqKeyStore> _keystore{new lxr::CoqKeyStore()};
    std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore()};
    std::shared_ptr<lxr::CoqFInfoStore> _finfostore{new lxr::CoqFInfoStore()};
    std::map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics1{};

    const int repetitions = 119834;
    const std::string m{"abcdef012345789ghijklmnopqrstuvwxyz"};
    std::string msg; //msg.reserve(m.size() * repetitions);
    for (int i = repetitions; i > 0; i--) {
        msg += m;
    }
    BOOST_CHECK_EQUAL(m.size() * repetitions, msg.size());
    lxr::Key256 _fhash = lxr::Sha3_256::hash(std::string("/etc/motd") + _config.my_id);
    const uint32_t block_sz = 32768;
    // 128 blocks; an assembly of 16 chunks holds 127 of them (16 * 262144 - 16 * 16 bytes)
    const uint32_t n_blocks = (msg.size() + block_sz - 1) / block_sz;
    BOOST_CHECK_EQUAL(128, n_blocks);

    // create CoqAssemblyCache and store a string
    {
        lxr::CoqAssemblyCache qac(_config, 3, 12);
        qac.register_key_store(_keystore);
        qac.register_fblock_store(_fblockstore);
        qac.register_finfo_store(_finfostore);

        auto addr_fblockstore1 = _fblockstore.get();

        lxr::WriteQueueEntity wqe;
        wqe._fhash = _fhash;
        auto msg_len = msg.size();
        uint32_t written{0};
        while (written < msg_len) {
            wqe._fpos = written;
            uint32_t sz = std::min((size_t)block_sz, msg_len - written);
            wqe._buffer.reset(new lxr::CoqBufferPlain(sz, msg.c_str() + written));
            written += sz;
            if (! qac.enqueue_write_request(wqe)) {
                auto wrote = qac.iterate_write_queue();
                qac.enqueue_write_request(wqe);
            }
        }
        std::clog << " QAC: blocks: " << _fblockstore->size() << std::endl;
        qac.iterate_write_queue();
        std::clog << " QAC: blocks: " << _fblockstore->size() << std::endl;
        qac.close();
        auto addr_fblockstore2 = _fblockstore.get();
        BOOST_CHECK_EQUAL(addr_fblockstore1, addr_fblockstore2);

        std::clog << " QAC: blocks: " << _fblockstore->size() << std::endl;
        BOOST_CHECK_EQUAL(_keystore->size(), 2);
        BOOST_CHECK_EQUAL(qac.get_fblock_store()->size(), 1);
        auto bp = qac.get_fblock_store()->at(0);
        BOOST_CHECK_EQUAL(bp->second->size(), n_blocks);
        BOOST_CHECK_EQUAL(_fblockstore->size(), 1);

        auto metrics = qac.metrics();
        map_metrics1.insert(metrics.begin(), metrics.end());

    }

    // ----


    for (auto const &e : map_metrics1) {
        std::clog << "  " << e.first << ": ";
        if (std::holds_alternative<std::string>(e.second))
            std::clog << std::get<std::string>(e.second) << std::endl;
        if (std::holds_alternative<uint32_t>(e.second))
            std::clog << std::get<uint32_t>(e.second) << std::endl;
        if (std::holds_alternative<uint64_t>(e.second))
            std::clog << std::get<uint64_t>(e.second) << std::endl;
    }

    BOOST_CHECK_EQUAL(m.size() * repetitions, std::get<uint64_t>(map_metrics1["backup_bytes"]));
    BOOST_CHECK_EQUAL(0ULL, std::get<uint64_t>(map_metrics1["restored_bytes"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["n_read_environments"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_write_environments"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_assembly_finalised"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["n_assembly_failed"]));
    BOOST_CHECK_EQUAL(n_blocks, std::get<uint32_t>(map_metrics1["n_write_requests"]));
    // the queue holds 12 requests
    BOOST_CHECK_EQUAL((n_blocks - 1) / 12 + 1, std::get<uint32_t>(map_metrics1["n_iterate_writes"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["len_read_queue"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["len_write_queue"]));


    // create CoqAssemblyCache and retrieve a string
    {
        lxr::CoqAssemblyCache qac(_config, 3, 12);
        qac.register_key_store(_keystore);
        qac.register_fblock_store(_fblockstore);
        qac.register_finfo_store(_finfostore);

        lxr::ReadQueueEntity rqe;
        int bidx = 13;
        auto bp = _fblockstore->at(0);
        BOOST_CHECK_EQUAL(bp->second->size(), n_blocks);
        lxr::CoqAssembly::BlockInformation bi = bp->second->at(bidx);
        BOOST_CHECK_EQUAL(block_sz, bi.blocksize);
        BOOST_CHECK_EQUAL(bidx * block_sz, bi.filepos);
        rqe._aid = bi.blockaid;
        rqe._apos = bi.blockapos;
        rqe._rlen = bi.blocksize;
        rqe._fpos = bi.filepos;
        rqe._chksum = bi.bchecksum;

        qac.enqueue_read_request(rqe);
        auto rres = qac.iterate_read_queue();
        qac.close();

        BOOST_REQUIRE_EQUAL(1, rres.size());
        BOOST_REQUIRE(rres[0]._rresult);
        auto chk = rres[0]._rresult->calc_checksum();
        BOOST_CHECK_EQUAL(chk->toHex(), bi.bchecksum.toHex());
    }

    // alternate between the two assemblies with a cache of one environment
    {
        lxr::CoqAssemblyCache qac(_config, 1, 12);
        qac.register_key_store(_keystore);
        qac.register_fblock_store(_fblockstore);
        qac.register_finfo_store(_finfostore);

        auto bp = _fblockstore->at(0);
        const auto & bis = *(bp->second);
        BOOST_CHECK_NE(bis.at(0).blockaid, bis.at(127).blockaid);
        auto mk_rqe = [](const lxr::CoqAssembly::BlockInformation & bi) {
            lxr::ReadQueueEntity rqe;
            rqe._aid = bi.blockaid; rqe._apos = bi.blockapos;
            rqe._rlen = bi.blocksize; rqe._fpos = bi.filepos;
            rqe._chksum = bi.bchecksum;
            return rqe;
        };
        for (int idx : {0, 127, 1, 127, 2}) {
            qac.enqueue_read_request(mk_rqe(bis.at(idx)));
        }
        // an unknown assembly stops the processing of the queue
        auto unknown = mk_rqe(bis.at(3));
        unknown._aid = "unknown";
        qac.enqueue_read_request(unknown);
        qac.enqueue_read_request(mk_rqe(bis.at(4)));
        // a wrong checksum gives no result for this block
        auto rres = qac.iterate_read_queue();
        BOOST_REQUIRE_EQUAL(5, rres.size());
        for (auto const & rr : rres) {
            BOOST_REQUIRE(rr._rresult);
            BOOST_CHECK_EQUAL(rr._rresult->calc_checksum()->toHex(), rr._readrequest._chksum.toHex());
        }
        auto wrong = mk_rqe(bis.at(5));
        wrong._chksum = lxr::Sha3_256::hash(std::string("wrong"));
        qac.enqueue_read_request(wrong);
        rres = qac.iterate_read_queue();
        BOOST_REQUIRE_EQUAL(1, rres.size());
        BOOST_CHECK(! rres[0]._rresult);

        auto metrics = qac.metrics();
        std::map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics{};
        map_metrics.insert(metrics.begin(), metrics.end());
        BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics["n_read_environments"]));
        BOOST_CHECK_EQUAL(5UL, std::get<uint32_t>(map_metrics["n_assembly_recalled"]));
    }
}

BOOST_AUTO_TEST_SUITE_END()
