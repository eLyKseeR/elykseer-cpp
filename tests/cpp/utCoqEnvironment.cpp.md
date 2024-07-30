```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/coqenvironment.hpp"
#include "lxr/coqstore.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utEnvironment

on class [CoqEnvironment](../src/coqenvironment.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utCoqEnvironment )
```

## Test case: create a _CoqEnvironmentWritable_

an empty assembly is not encrypted and extracted.

```cpp
BOOST_AUTO_TEST_CASE( instantiate_CoqEnvironmentWritable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqEnvironmentWritable _env(_config);
    auto keys = _env.finalise_assembly();

    BOOST_CHECK(! keys);
}
```

## Test case: create a _CoqEnvironmentReadable_

```cpp
BOOST_AUTO_TEST_CASE( instantiate_CoqEnvironmentReadable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqEnvironmentReadable _env(_config);
    // auto keys = _env.finalise_assembly();

    // BOOST_CHECK(! keys);
}
```

## Test case: roundtrip of backup, finish, encrypt, extract, recall, decrypt, restore

```cpp
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

    std::shared_ptr<lxr::CoqFBlockStore> _fblockstore{new lxr::CoqFBlockStore(_config)};
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
    _renv.restore_assembly(_keys.value().first, _keys.value().second);

    // test restored bytes
    lxr::CoqAssembly::BlockInformation bi = _fblockstore->at(0)->second.front();
    BOOST_CHECK_EQUAL(bi.blocksize, msg.length());
    std::shared_ptr<lxr::CoqBufferPlain> restored_block = _renv._assembly->restore(bi);
    char restored_buffer[bi.blocksize];
    restored_block->to_buffer(bi.blocksize, restored_buffer);
    BOOST_CHECK_EQUAL(std::string(restored_buffer, bi.blocksize), msg);
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
