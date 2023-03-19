```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"
#include "rapidcheck/boost_test.h"

#include "lxr/dbfp.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/key128.hpp"
#include "lxr/key256.hpp"

#include "boost/chrono.hpp"
using clk = boost::chrono::high_resolution_clock;
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utDbFp

on class [DbFp](../src/dbfp.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utDbFp )
```
## Test case: set and get record
```cpp
BOOST_AUTO_TEST_CASE( set_get_record )
{
  const std::string fp1 = "/home/me/Documents/interesting.txt";
  const std::string fp2 = "/Data/photos/2001/motorcycle.jpeg";

  lxr::DbFp _db;
  auto _dat1 = lxr::DbFpDat::make(fp1);
  auto _dat2 = lxr::DbFpDat::make(fp2);
  _dat1->_len = 1378; _dat2->_len = 1749302;
  _dat1->_osusr = "me"; _dat1->_osgrp = "users";
  _dat2->_osusr = "nobody"; _dat2->_osgrp = "nobody";
  lxr::DbFpBlock _block1 {1, 193943, 0UL, (int)_dat1->_len, (int)_dat1->_len, false, {}, {}};
  _dat1->_blocks->push_back(std::move(_block1));
  int idx = 1;
  uint64_t len = 0ULL;
  int bs = (1<<16) - 1;
  while (len < _dat2->_len) {
    lxr::DbFpBlock _block2 {idx++, 7392+(int)len, len, std::min(bs,int(_dat2->_len-len)), std::min(bs,int(_dat2->_len-len)), false, {}, {}};
    _dat2->_blocks->push_back(std::move(_block2));
    len += bs;
  }
  _db.set(fp1, *_dat1);
  _db.set(fp2, *_dat2);
  auto ob1 = _db.get(fp1);
  auto ob2 = _db.get(fp2);
  BOOST_CHECK(ob1);
  BOOST_CHECK(ob2);
  BOOST_CHECK_EQUAL(2, _db.count());
  BOOST_CHECK_EQUAL(_dat1->_len, ob1->_len);
  BOOST_CHECK_EQUAL(_dat2->_len, ob2->_len);
}
```

## Test case: union of records
```cpp
BOOST_AUTO_TEST_CASE( union_record )
{
  const std::string fp1 = "/home/me/Documents/interesting.txt";
  const std::string fp2 = "/Data/photos/2001/motorcycle.jpeg";

  lxr::DbFp _db, _db1;
  auto _dat1 = lxr::DbFpDat::make(fp1);
  auto _dat2 = lxr::DbFpDat::make(fp2);
  _dat1->_len = 1378; _dat2->_len = 1749302;
  _dat1->_osusr = "me"; _dat1->_osgrp = "users";
  _dat2->_osusr = "nobody"; _dat2->_osgrp = "nobody";
  lxr::DbFpBlock _block1 {1, 193943, 0UL, (int)_dat1->_len, (int)_dat1->_len, false, {}, {}};
  _dat1->_blocks->push_back(std::move(_block1));
  int idx = 1;
  uint64_t len = 0ULL;
  int bs = (1<<16) - 1;
  while (len < _dat2->_len) {
    lxr::DbFpBlock _block2 {idx++, 7392+(int)len, len, std::min(bs,int(_dat2->_len-len)), std::min(bs,int(_dat2->_len-len)), false, {}, {}};
    _dat2->_blocks->push_back(std::move(_block2));
    len += bs;
  }
  _db.set(fp1, *_dat1);
  _db1.set(fp2, *_dat2);
  _db.unionWith(_db1);
  auto ob1 = _db.get(fp1);
  auto ob2 = _db.get(fp2);
  BOOST_CHECK(ob1);
  BOOST_CHECK(ob2);
  BOOST_CHECK(_db.contains(fp1));
  BOOST_CHECK(_db.contains(fp2));
  BOOST_CHECK(_db1.contains(fp2));
  BOOST_CHECK_EQUAL(2, _db.count());
  BOOST_CHECK_EQUAL(_dat1->_len, ob1->_len);
  BOOST_CHECK_EQUAL(_dat2->_len, ob2->_len);
}
```

## Test case: output to XML file
```cpp
/* DISABLED
BOOST_AUTO_TEST_CASE( output_to_xml )
{
  const std::string fp1 = "/home/me/Documents/interesting.txt";
  const std::string fp2 = "/Data/photos/2001/motorcycle.jpeg";

  lxr::DbFp _db;
  lxr::DbFpDat _dat1 = lxr::DbFpDat::make(fp1);
  lxr::DbFpDat _dat2 = lxr::DbFpDat::make(fp2);
  _dat1._len = 1378; _dat2._len = 1749302;
  _dat1._osusr = "me"; _dat1._osgrp = "users";
  _dat2._osusr = "nobody"; _dat2._osgrp = "nobody";
  lxr::DbFpBlock _block1 {1, 193943, 0UL, (int)_dat1._len, (int)_dat1._len, false, {}, {}};
  _dat1._blocks->push_back(_block1);
  int idx = 1;
  uint64_t len = 0ULL;
  int bs = (1<<16) - 1;
  while (len < _dat2._len) {
    lxr::DbFpBlock _block2 {idx++, 7392+(int)len, len, std::min(bs,int(_dat2._len-len)), std::min(bs,int(_dat2._len-len)), false, {}, {}};
    _dat2._blocks->push_back(_block2);
    len += bs;
  }
  _db.set(fp1, _dat1);
  _db.set(fp2, _dat2);
  auto ob1 = _db.get(fp1);
  auto ob2 = _db.get(fp2);
  BOOST_CHECK(ob1);
  BOOST_CHECK(ob2);
  BOOST_CHECK_EQUAL(2, _db.count());
  auto const tmpd = std::filesystem::temp_directory_path();
  auto const fp_xml = tmpd / "test_dbfp_1.xml";
  std::ofstream _outs; _outs.open(fp_xml.native());
  _db.outStream(_outs);
} */
```

## Test case: input from XML file
```cpp
BOOST_AUTO_TEST_CASE( input_from_xml )
{
  const std::string fp1 = "/home/me/Documents/interesting.txt";
  const std::string fp2 = "/Data/photos/2001/motorcycle.jpeg";

  lxr::Key256 _aid1, _aid2;
  lxr::DbFp _db;
  auto _dat1 = lxr::DbFpDat::make(fp1);
  auto _dat2 = lxr::DbFpDat::make(fp2);
  _dat1->_len = 1378; _dat2->_len = 1749302;
  _dat1->_osusr = "me"; _dat1->_osgrp = "users";
  _dat2->_osusr = "nobody"; _dat2->_osgrp = "nobody";
  lxr::DbFpBlock _block1 {1, 193943, 0UL, (int)_dat1->_len, (int)_dat1->_len, false, {}, _aid1.toHex()};
  _dat1->_blocks->push_back(std::move(_block1));
  int idx = 1;
  uint64_t len = 0ULL;
  int bs = (1<<16) - 1;
  while (len < _dat2->_len) {
    lxr::DbFpBlock _block2 {idx++, 7392+(int)len, len, std::min(bs,int(_dat2->_len-len)), std::min(bs,int(_dat2->_len-len)), false, {}, _aid2.toHex()};
    _dat2->_blocks->push_back(std::move(_block2));
    len += bs;
  }
  _db.set(fp1, std::move(*_dat1));
  _db.set(fp2, std::move(*_dat2));

  std::string _buf;
  std::ostringstream _outs;
  _db.outStream(_outs); _outs.flush();
  _buf = _outs.str();

  //std::cout << _buf << std::endl;

  std::istringstream _ins(_buf);

  lxr::DbFp _db2;
  _db2.inStream(_ins);

  std::ostringstream _outs2;
  _db2.outStream(_outs2); _outs2.flush();
  // std::string _buf2 = _outs2.str();
  //std::cout << _buf2 << std::endl;

  auto const _fpdat11 = _db.get(fp1);
  auto const _fpdat12 = _db.get(fp2);
  auto const _fpdat21 = _db2.get(fp1);
  auto const _fpdat22 = _db2.get(fp2);

  BOOST_CHECK(_fpdat11); BOOST_CHECK(_fpdat12);
  BOOST_CHECK(_fpdat21); BOOST_CHECK(_fpdat22);
  BOOST_CHECK_EQUAL(*_fpdat11, *_fpdat21);
  BOOST_CHECK_EQUAL(*_fpdat12, *_fpdat22);
}
```

## Test case: instantiate on real file
```cpp
BOOST_AUTO_TEST_CASE( instantiate_from_file )
{
#if defined(_WIN32)
  const std::string fp = "\\Windows\\cmd.exe";
#else
  const std::string fp = "/bin/sh";
#endif

  auto _entry = lxr::DbFpDat::fromFile(fp);

  BOOST_CHECK( _entry->_len > 0 );
  BOOST_CHECK_EQUAL( "0", _entry->_osusr);
  BOOST_CHECK_EQUAL( "0", _entry->_osgrp);
  BOOST_CHECK( _entry->_osattr.size() > 0 );
  //std::clog << "mtime: " << _entry->_osattr << std::endl;
}
```

## Time complexity of DbFp depending on size
```cpp
RC_BOOST_PROP( roundtrip, () )
{
  const std::string fp = "/home/me/Documents/important.dat";
  lxr::DbFp _db;
  auto _dat = lxr::DbFpDat::make(fp);
  auto const tgtlen = *rc::gen::inRange(101, 1000002);
  _dat->_len = tgtlen;
  _dat->_osusr = "me"; _dat->_osgrp = "users";
  int idx = 1;
  uint64_t len = 0ULL;
  int bs = (1<<16) - 1;
  while (len < _dat->_len) {
    lxr::DbFpBlock _block {idx++, 7392+(int)len, len, std::min(bs,int(_dat->_len-len)), std::min(bs,int(_dat->_len-len)), false, {}, {}};
    _dat->_blocks->push_back(std::move(_block));
    len += bs;
  }
  auto const nblocks = _dat->_blocks->size();
  auto const ndatlen = _dat->_len;
  _db.set(fp, std::move(*_dat));
  RC_ASSERT(1 == _db.count());

  std::string _buf;
  std::ostringstream _outs;
  _db.outStream(_outs); _outs.flush();
  _buf = _outs.str();

  lxr::DbFp _db2;
  auto t0 = clk::now();
  std::istringstream _ins(_buf);
  _db2.inStream(_ins); //_ins.close();
  RC_ASSERT(1 == _db2.count());
  auto t1 = clk::now();
  auto tdiff = boost::chrono::round<boost::chrono::microseconds>(t1 - t0).count();
  RC_TAG(std::pair(nblocks, tdiff));

  auto obj = _db.get(fp);
  RC_ASSERT(obj->_len == ndatlen);
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
