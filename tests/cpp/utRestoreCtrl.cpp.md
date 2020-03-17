```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/restorectrl.hpp"
#include "lxr/options.hpp"

#include <iostream>
#include <fstream>
````

# Test suite: utRestoreCtrl

on class [BackupCtrl](../src/restorectrl.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utRestoreCtrl )
```
## Test case: startup invariants
```cpp
BOOST_AUTO_TEST_CASE( check_startup )
{
    lxr::RestoreCtrl _ctrl;

	BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
	BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());
}
```

## Test case: decrypt a file
```cpp
BOOST_AUTO_TEST_CASE( decrypt_file )
{
    lxr::Options::set().isCompressed(true);
    lxr::Options::set().nChunks(16);
    lxr::Options::set().fpathChunks() = "/tmp/LXR";
    lxr::Options::set().fpathMeta() = "/tmp/meta";
    lxr::RestoreCtrl _ctrl;

    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());

    lxr::DbFp _dbfp;
    { std::ifstream _if; _if.open("/tmp/test_dbfp_backup.xml");
      _dbfp.inStream(_if); _if.close(); }
    _ctrl.addDbFp(_dbfp);
    lxr::DbKey _dbks;
    { std::ifstream _if; _if.open("/tmp/test_dbkey_backup.xml");
      _dbks.inStream(_if); _if.close(); }
    _ctrl.addDbKey(_dbks);
    const std::string _fp = "/tmp/test_data_file";
    
    BOOST_REQUIRE( _ctrl.restore("/tmp/", _fp) );

    BOOST_CHECK( _ctrl.bytes_in() > 0);
    BOOST_CHECK( _ctrl.bytes_out() > 0);

}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
