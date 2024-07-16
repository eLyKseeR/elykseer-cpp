```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/coqassemblycache.hpp"
#include "lxr/coqstore.hpp"
#include "lxr/key256.hpp"
#include "lxr/sha3.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utCoqAssemblyCache

on class [CoqAssemblyCache](../src/coqassemblycache.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utCoqAssemblyCache )
```

## Test case: empty metrics on new _CoqAssemblyCache_

```cpp
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
```

## Test case: backup some data

```cpp
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

    std::shared_ptr<lxr::CoqKeyStore> _keystore{new lxr::CoqKeyStore(_config)};
    std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore(_config)};
    std::shared_ptr<lxr::CoqFInfoStore> _finfostore{new lxr::CoqFInfoStore(_config)};
    std::map<std::string, lxr::CoqAssemblyCache::mvalue_t> map_metrics1{};

    std::string msg{"abcdef012345789"};
    lxr::Key256 _fhash = lxr::Sha3_256::hash(std::string("/etc/motd"));

    // create CoqAssemblyCache and store a string
    {
        lxr::CoqAssemblyCache qac(_config, 3, 12);
        qac.register_key_store(_keystore);
        qac.register_fblock_store(_fblockstore);
        qac.register_finfo_store(_finfostore);

        lxr::WriteQueueEntity wqe;
        wqe._fhash = _fhash;
        wqe._fpos = 42;
        wqe._buffer.reset(new lxr::CoqBufferPlain(15, msg.c_str()));

        qac.enqueue_write_request(wqe);
        auto wrote = qac.iterate_write_queue();
        qac.close();

        BOOST_CHECK_EQUAL(_keystore->size(), 1);
        BOOST_CHECK_EQUAL(_fblockstore->size(), 1);

        auto metrics = qac.metrics();
        map_metrics1.insert(metrics.begin(), metrics.end());

    }

    // ----

    lxr::CoqAssembly::aid_t _aid = _keystore->at(0)->first;

    for (auto const &e : map_metrics1) {
        std::clog << "  " << e.first << ": ";
        if (std::holds_alternative<std::string>(e.second))
            std::clog << std::get<std::string>(e.second) << std::endl;
        if (std::holds_alternative<uint32_t>(e.second))
            std::clog << std::get<uint32_t>(e.second) << std::endl;
        if (std::holds_alternative<uint64_t>(e.second))
            std::clog << std::get<uint64_t>(e.second) << std::endl;
    }

    BOOST_CHECK_EQUAL(15ULL, std::get<uint64_t>(map_metrics1["backup_bytes"]));
    BOOST_CHECK_EQUAL(0ULL, std::get<uint64_t>(map_metrics1["restored_bytes"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["n_read_environments"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_write_environments"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_assembly_finalised"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_write_requests"]));
    BOOST_CHECK_EQUAL(1UL, std::get<uint32_t>(map_metrics1["n_iterate_writes"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["len_read_queue"]));
    BOOST_CHECK_EQUAL(0UL, std::get<uint32_t>(map_metrics1["len_write_queue"]));


    // create CoqAssemblyCache and retrieve a string
    {
        lxr::CoqAssemblyCache qac(_config, 3, 12);
        qac.register_key_store(_keystore);
        qac.register_fblock_store(_fblockstore);
        qac.register_finfo_store(_finfostore);

        lxr::ReadQueueEntity rqe;
        rqe._aid = _aid;
        lxr::CoqAssembly::BlockInformation bi = _fblockstore->at(0)->second.front();
        rqe._apos = bi.blockapos;
        rqe._rlen = msg.length();

        qac.enqueue_read_request(rqe);
        auto rres = qac.iterate_read_queue();
        qac.close();

        BOOST_CHECK_EQUAL(1, rres.size());
        char restored_buffer[bi.blocksize];
        rres[0]._rresult->to_buffer(bi.blocksize, restored_buffer);
        BOOST_CHECK_EQUAL(std::string(restored_buffer, bi.blocksize), msg);
    }
}
```


```cpp
BOOST_AUTO_TEST_SUITE_END()
```
