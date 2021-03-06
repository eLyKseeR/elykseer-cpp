```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/dbkey.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/key128.hpp"
#include "lxr/key256.hpp"

#include <iostream>
#include <fstream>
````

# Test suite: utDbKey

on class [DbKey](../src/dbkey.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utDbKey )
```
## Test case: set and get record
```cpp
BOOST_AUTO_TEST_CASE( set_get_record )
{
    const std::string aid1 = "94ffed38da8acee6f14be5af8a31d3b8015e008f9b30c7d12a58bfed57ba8d12";
	const std::string aid2 = "a31d3b8015e00894ffed38da8acee6f14be5af8f9b30c7d12a58bfed57ba8d12";
	
	lxr::DbKey _db;
	lxr::DbKeyBlock _block1;  _block1._n=64;
	lxr::DbKeyBlock _block2;  _block2._n=16;
	_db.set(aid1, _block1);
	_db.set(aid2, _block2);
	auto ob1 = _db.get(aid1);
	auto ob2 = _db.get(aid2);
	BOOST_CHECK(ob1);
	BOOST_CHECK(ob2);
	BOOST_CHECK_EQUAL(2, _db.count());
	BOOST_CHECK_EQUAL(16, ob2->_n);
	BOOST_CHECK_EQUAL(64, ob1->_n);
}
```

```cpp
static char _k1[64], _k2[64];
static char _iv1[32], _iv2[32];
```

## Test case: output to XML file
```cpp
BOOST_AUTO_TEST_CASE( output_to_xml )
{
    const std::string aid1 = "94ffed38da8acee6f14be5af8a31d3b8015e008f9b30c7d12a58bfed57ba8d12";
	const std::string aid2 = "a31d3b8015e00894ffed38da8acee6f14be5af8f9b30c7d12a58bfed57ba8d12";
	
	lxr::Key128 iv1, iv2;
	lxr::Key256 key1, key2;
	lxr::DbKey _db;
	lxr::DbKeyBlock _block1;  _block1._n=64; _block1._key=key1; _block1._iv=iv1;
	lxr::DbKeyBlock _block2;  _block2._n=16; _block2._key=key2; _block2._iv=iv2;
	_db.set(aid1, _block1);
	_db.set(aid2, _block2);
	BOOST_CHECK_EQUAL(2, _db.count());
	auto const tmpd = boost::filesystem::temp_directory_path();
	auto const fp_xml = tmpd / "test_dbkey_1.xml";
	std::ofstream _outs; _outs.open(fp_xml.native());
	_db.outStream(_outs);
	strncpy(_k1, key1.toHex().c_str(), 64);
	strncpy(_k2, key2.toHex().c_str(), 64);
	strncpy(_iv1, iv1.toHex().c_str(), 32);
	strncpy(_iv2, iv2.toHex().c_str(), 32);
}
```

## Test case: input from XML file
```cpp
BOOST_AUTO_TEST_CASE( input_from_xml )
{
    const std::string aid1 = "94ffed38da8acee6f14be5af8a31d3b8015e008f9b30c7d12a58bfed57ba8d12";
	const std::string aid2 = "a31d3b8015e00894ffed38da8acee6f14be5af8f9b30c7d12a58bfed57ba8d12";
	
	auto const tmpd = boost::filesystem::temp_directory_path();
	lxr::DbKey _db;
	auto const fp_xml = tmpd / "test_dbkey_1.xml";
	std::ifstream _ins; _ins.open(fp_xml.native());
	_db.inStream(_ins);
	BOOST_CHECK_EQUAL(2, _db.count());
	auto const keys1 = _db.get(aid1);
	auto const keys2 = _db.get(aid2);
	BOOST_CHECK_EQUAL(std::string(_k1,64), keys1->_key.toHex());
	BOOST_CHECK_EQUAL(std::string(_k2,64), keys2->_key.toHex());
	BOOST_CHECK_EQUAL(std::string(_iv1,32), keys1->_iv.toHex());
	BOOST_CHECK_EQUAL(std::string(_iv2,32), keys2->_iv.toHex());
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
