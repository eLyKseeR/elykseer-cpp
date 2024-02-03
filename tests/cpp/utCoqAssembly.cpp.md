```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/coqassembly.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utCoqAssembly

on class [CoqAssembly](../src/coqassembly.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utCoqAssembly )
```

## Test case: cannot directly instantiate CoqAssembly

because _CoqAssembly_ is an abstract class we cannot create an instance of it.
expect this test to fail if the _#ifdef_ is commented out.

```cpp
#ifdef CHECK_EXPECTED_COMPILER_ERRORS
BOOST_AUTO_TEST_CASE( cannot_instantiate_CoqAssembly_directly )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssembly _assembly(_config);

    BOOST_CHECK(_assembly.assemblysize(16) > 0);
}
#endif
```

## Test case: create a _CoqAssemblyPlainWritable_

```cpp
BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyPlainWritable )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyPlainWritable _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(16, _assembly.apos());
}
```

## Test case: create a _CoqAssemblyPlainFull_

```cpp
BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyPlainFull )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyPlainFull _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(0, _assembly.apos());
}
```

## Test case: create a _CoqAssemblyEncrypted_

```cpp
BOOST_AUTO_TEST_CASE( instantiate_CoqAssemblyEncrypted )
{
    lxr::CoqConfiguration _config;
    lxr::CoqAssemblyEncrypted _assembly(_config);

    BOOST_CHECK_EQUAL(_assembly.buffer_len(), 16 * lxr::CoqAssembly::chunksize);
    BOOST_CHECK_EQUAL(0, _assembly.apos());
}
```

## Test case: roundtrip of backup, finish, encrypt, extract, recall, decrypt, restore

```cpp
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
    std::clog << "ivec: " << ki._pkey.toHex() << std::endl;
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
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
