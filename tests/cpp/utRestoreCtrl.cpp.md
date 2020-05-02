```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/backupctrl.hpp"
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
BOOST_AUTO_TEST_CASE( encrypt_decrypt_file )
{
  // encrypt file
  {
    lxr::Options::set().isCompressed(false);
    lxr::Options::set().nChunks(16);
    lxr::Options::set().fpathChunks() = "/tmp/LXR";
    lxr::Options::set().fpathMeta() = "/tmp/meta";
    lxr::BackupCtrl _backup;

    const std::string _fdata = "/tmp/test_data_file";
    { std::ofstream _fe; _fe.open(_fdata);
     for (int i=0; i<9999; i++) { _fe.write("0123456789",10); }
     _fe.close();
    }

    BOOST_REQUIRE( _backup.backup(_fdata) );

    BOOST_CHECK( _backup.bytes_in() > 0);
    BOOST_CHECK( _backup.bytes_out() > 0);

    _backup.finalize();

    const std::string _fpath1 = "/tmp/test_dbfp_backup.xml";
    std::ofstream _out1; _out1.open(_fpath1);
    _backup.getDbFp().outStream(_out1);
    _out1.close();
    const std::string _fpath2 = "/tmp/test_dbkey_backup.xml";
    std::ofstream _out2; _out2.open(_fpath2);
    _backup.getDbKey().outStream(_out2);
    _out2.close();
  }

  // decrypt file
  {
    lxr::Options::set().isCompressed(false);
    lxr::Options::set().nChunks(16);
    lxr::Options::set().fpathChunks() = "/tmp/LXR";
    lxr::Options::set().fpathMeta() = "/tmp/meta";
    lxr::RestoreCtrl _restore;

    BOOST_CHECK_EQUAL(0UL, _restore.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _restore.bytes_out());

    lxr::DbFp _dbfp;
    { std::ifstream _if; _if.open("/tmp/test_dbfp_backup.xml");
      _dbfp.inStream(_if); _if.close(); }
    _restore.addDbFp(_dbfp);
    lxr::DbKey _dbks;
    { std::ifstream _if; _if.open("/tmp/test_dbkey_backup.xml");
      _dbks.inStream(_if); _if.close(); }
    _restore.addDbKey(_dbks);
    const std::string _fp = "/tmp/test_data_file";
    
    BOOST_REQUIRE( _restore.restore("/tmp/", _fp) );

    BOOST_CHECK( _restore.bytes_in() > 0);
    BOOST_CHECK( _restore.bytes_out() > 0);
  }
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
