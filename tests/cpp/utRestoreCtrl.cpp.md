```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/backupctrl.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
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

The test may fail on firt execution because the output directory
`$TMPDIR/restored` does not exist.

```cpp
BOOST_AUTO_TEST_CASE( backup_restore_file )
{
  auto const tmpd = boost::filesystem::temp_directory_path();
  lxr::Options::set().fpathChunks() = tmpd / "LXR";
  lxr::Options::set().fpathMeta() = tmpd / "meta";
  BOOST_REQUIRE_MESSAGE( lxr::FileCtrl::dirExists(lxr::Options::current().fpathMeta()), "missing directory: meta" );

  auto const outputdir = tmpd / "restored";
  BOOST_REQUIRE_MESSAGE( lxr::FileCtrl::dirExists(outputdir), "missing directory: restored" );
  const std::string datafname = "test_data_file";
  const std::string datafile = (tmpd / datafname).native();
  auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_restore.xml";
  auto const fp_dbky = lxr::Options::current().fpathMeta() / "test_dbkey_restore.xml";

  // cleanup
  {
    auto restoredfp = outputdir / datafile;
    if (boost::filesystem::exists(restoredfp)) {
      boost::filesystem::remove(restoredfp);
    }
  }

  // encrypt file
  {
    lxr::Options::set().isCompressed(true);
    lxr::Options::set().nChunks(16);
    lxr::BackupCtrl _backup;

    { std::ofstream _fe; _fe.open(datafile);
     for (int i=0; i<9999; i++) { _fe.write("0123456789",10); }
     _fe.close();
    }

    BOOST_REQUIRE( _backup.backup(datafile) );

    BOOST_CHECK( _backup.bytes_in() > 0);
    BOOST_CHECK( _backup.bytes_out() > 0);
    BOOST_CHECK( _backup.bytes_in() > _backup.bytes_out());  // by compression

    _backup.finalize();

    std::ofstream _out1; _out1.open(fp_dbfp.native());
    _backup.getDbFp().outStream(_out1);
    _out1.close();
    std::ofstream _out2; _out2.open(fp_dbky.native());
    _backup.getDbKey().outStream(_out2);
    _out2.close();
  }

  // decrypt file
  {
    lxr::Options::set().isCompressed(false); // overwritten from meta data
    lxr::Options::set().nChunks(16);
    lxr::RestoreCtrl _restore;

    BOOST_CHECK_EQUAL(0UL, _restore.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _restore.bytes_out());

    lxr::DbFp _dbfp;
    { std::ifstream _if; _if.open(fp_dbfp.native());
      _dbfp.inStream(_if); _if.close(); }
    BOOST_CHECK_EQUAL(1, _dbfp.count());
    _restore.addDbFp(_dbfp);
    lxr::DbKey _dbks;
    { std::ifstream _if; _if.open(fp_dbky.native());
      _dbks.inStream(_if); _if.close(); }
    BOOST_CHECK_EQUAL(1, _dbks.count());
    _restore.addDbKey(_dbks);

    BOOST_REQUIRE( _restore.restore(outputdir, datafile) );

    BOOST_CHECK( _restore.bytes_in() > 0);
    BOOST_CHECK( _restore.bytes_out() > 0);
    BOOST_CHECK( _restore.bytes_out() > _restore.bytes_in());  // by compression
  }
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
