```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"
#include "boost/contract.hpp"

#include "lxr/assembly.hpp"
#include "lxr/chunk.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/key256.hpp"
#include "lxr/key128.hpp"
#include "lxr/md5.hpp"
#include "lxr/options.hpp"
#include "sizebounded/sizebounded.ipp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utAssembly

on class [Assembly](../src/assembly.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utAssembly )
```

## Test case: state of assembly
```cpp
BOOST_AUTO_TEST_CASE( assembly_state_after_init )
{
  lxr::Assembly _a1(16);

  BOOST_CHECK(_a1.isWritable());
#ifdef DEBUG
  BOOST_CHECK(_a1.isReadable());  // temporarily disabled
#else
  BOOST_CHECK(! _a1.isReadable());
#endif
  BOOST_CHECK(! _a1.isEncrypted());
}
```

## Test case: state of assembly
```cpp
BOOST_AUTO_TEST_CASE( assembly_creation_failed )
{
  boost::contract::set_precondition_failure (
        [] (boost::contract::from where) {
            throw; // Re-throw (assertion_failure, user-defined, etc.).
        }
    );

  BOOST_CHECK_THROW(lxr::Assembly _a1(1), boost::contract::assertion_failure);
}
```

## Test case: a new assembly contains random  bytes (128/8 = 16 bytes)
```cpp
BOOST_AUTO_TEST_CASE( initialised_assembly_with_random_bytes )
{
  lxr::Assembly _a1(17);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  BOOST_CHECK_EQUAL(16, _a1.pos());
}
```

## Test case: access buffer denied if unencrypted
```cpp
#ifdef DEBUG
#warning disabled test 'access_buffer_denied'
#else
BOOST_AUTO_TEST_CASE( access_buffer_denied )
{
  lxr::Assembly _a1(16);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  // cannot read from unencrypted buffer
  BOOST_CHECK_EQUAL(0, _a1.getData(0, 10, buf));
}
#endif
```

## Test case: access buffer if encrypted
```cpp
BOOST_AUTO_TEST_CASE( access_buffer_allowed )
{
  lxr::Assembly _a1(17);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  lxr::Key256 _k;
  lxr::Key128 _iv;
  BOOST_CHECK( _a1.encrypt(_k, _iv) );
  BOOST_CHECK_EQUAL(11, _a1.getData(0, 10, buf));
}
```

## Test case: encrypt then decrypt assembly
```cpp
BOOST_AUTO_TEST_CASE( assembly_encrypt_then_decrypt )
{
  const std::string msg = "all my precious data are save, so I will sleep fine!";

  lxr::Assembly _a1(23);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  buf.transform([&msg](int i, unsigned char c)->unsigned char {
      if (i < msg.size()) {
          return msg[i];
      } else {
          return '\\0';
      }
  });
  _a1.addData(msg.size(), buf);
  lxr::Key256 _k;
  lxr::Key128 _iv;
  BOOST_CHECK( _a1.encrypt(_k, _iv) );
 
  BOOST_CHECK(! _a1.isWritable());
  BOOST_CHECK(_a1.isReadable());
  BOOST_CHECK(_a1.isEncrypted());

  buf.transform([](int i, unsigned char c)->unsigned char {
      return '\\0';
  });
  BOOST_CHECK_EQUAL(10, _a1.getData(16,16+9,buf));
  BOOST_CHECK(buf.toString().substr(0,9) != msg.substr(0,9));

  BOOST_CHECK(  _a1.decrypt(_k, _iv) );
  BOOST_CHECK(! _a1.isWritable() );
  BOOST_CHECK(  _a1.isReadable() );
  BOOST_CHECK(! _a1.isEncrypted() );
  buf.transform([](int i, unsigned char c)->unsigned char {
      return '\\0';
  });
  BOOST_CHECK_EQUAL(10, _a1.getData(16,16+9,buf));
  BOOST_CHECK_EQUAL(buf.toString().substr(0,9), msg.substr(0,9));
}
```

## Test case: set and get data
```cpp
#ifdef DEBUG
BOOST_AUTO_TEST_CASE( assembly_set_get_data )
{
  const std::string msg = "0123456789abcdefghijklmnopqrstuvwxyz";
  lxr::Assembly _ass(21);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  const int msz = msg.size();
  buf.transform([&msg,msz](int i, unsigned char c)->unsigned char {
    return (unsigned char)msg[i % msz];
  });
  _ass.addData(buf.size(), buf);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf2;
  // can only access buffer when defined(DEBUG)
  BOOST_CHECK_EQUAL(_ass.getData(16, buf2.size()-1, buf2), buf2.size()-16);
  char s[msz+1];
  buf2.map([&s,msz](int i, unsigned char c) {
    if (i < msz) { s[i] = (char)c; }
  });
  s[msz] = '\0';
  const std::string msg2(s, msz);
  std::clog << "read back: " << msg2 << std::endl;
  BOOST_CHECK_EQUAL(msg, msg2);
}
#endif
```

## Test case: encrypt assembly then extract chunks
```cpp
BOOST_AUTO_TEST_CASE( assembly_encrypt_then_extract_chunks )
{
  const std::string msg = "0123456789abcdefghijklmnopqrstuvwxyz";

  auto const tmpd = std::filesystem::temp_directory_path();
  auto const outputpath = tmpd / "LXR";
  if (! std::filesystem::exists(outputpath)) {
    std::filesystem::create_directory(outputpath);
  }
  int nChunks = 17;
  lxr::Options::set().nChunks(nChunks)
                     .isCompressed(false)
                     .fpathChunks(outputpath);
  lxr::Assembly _a1(nChunks);
  sizebounded<unsigned char, lxr::Assembly::datasz> buf;
  const int msz = msg.size();
  buf.transform([&msg,msz](int i, unsigned char c)->unsigned char {
    return (unsigned char)msg[i % msz];
  });
  _a1.addData(buf.size()-16, buf);  // minus random bytes at beginning
  auto const md5_1 = lxr::Md5::hash((const char*)buf.ptr(), buf.size()-16);
  #ifdef DEBUG
  { std::ofstream fass; fass.open((tmpd / "test_assembly.filled").native());
    const int bufsz = 4096;
    sizebounded<unsigned char, bufsz> buf2;
    for (int i=0; i<nChunks()*lxr::Chunk::size; i+=bufsz) {
        BOOST_CHECK_EQUAL(_a1.getData(i, i+bufsz-1, buf2), bufsz);
        fass.write((const char*)buf2.ptr(),bufsz); }
    fass.close();
  }
  #endif

  lxr::Key256 _k;
  lxr::Key128 _iv;
  BOOST_CHECK( _a1.encrypt(_k, _iv) );
  BOOST_CHECK( _a1.isEncrypted() );
  BOOST_CHECK( _a1.extractChunks() );

  auto const aid = _a1.aid();
  lxr::Assembly _a2(aid, nChunks);
  BOOST_CHECK( _a2.insertChunks() );
  BOOST_CHECK( _a2.isEncrypted() );
  BOOST_CHECK( _a2.decrypt(_k, _iv) );
  BOOST_CHECK(!_a2.isEncrypted() );

  #ifdef DEBUG
  { std::ofstream fass; fass.open((tmpd / "test_assembly.decrypted").native());
    const int bufsz = 4096;
    sizebounded<unsigned char, bufsz> buf2;
    for (int i=0; i<nChunks()*lxr::Chunk::size; i+=bufsz) {
        BOOST_CHECK_EQUAL(_a2.getData(i, i+bufsz-1, buf2), bufsz);
        fass.write((const char*)buf2.ptr(),bufsz); }
    fass.close();
  }
  #endif
  
  sizebounded<unsigned char, lxr::Assembly::datasz> buf2;
  BOOST_CHECK_EQUAL(_a2.getData(16, buf2.size()-1, buf2), buf2.size()-16);   // same length as input
  auto const md5_2 = lxr::Md5::hash((const char*)buf2.ptr(), buf2.size()-16);
  BOOST_CHECK_EQUAL(md5_1, md5_2);
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
