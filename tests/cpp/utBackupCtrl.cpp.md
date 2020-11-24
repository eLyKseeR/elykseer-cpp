```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/backupctrl.hpp"
#include "lxr/chunk.hpp"
#include "lxr/options.hpp"

#include <iostream>
#include <fstream>
````

# Test suite: utBackupCtrl

on class [BackupCtrl](../src/backupctrl.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utBackupCtrl )
```
## Test case: startup invariants
```cpp
BOOST_AUTO_TEST_CASE( check_startup )
{
    int nchunks = 21;
    lxr::Options::set().nChunks(nchunks);
    lxr::BackupCtrl _ctrl;

    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());
    BOOST_CHECK_EQUAL(0UL, _ctrl.free());
}
```

## Test case: encrypt a file with compression
```cpp
BOOST_AUTO_TEST_CASE( encrypt_file_compressed )
{
    auto const tmpd = boost::filesystem::temp_directory_path();
    lxr::Options::set().nChunks(32);
    lxr::Options::set().isCompressed(true);
    lxr::Options::set().fpathChunks() = tmpd / "LXR";
    lxr::Options::set().fpathMeta() = tmpd /"meta";
    lxr::BackupCtrl _ctrl;

    BOOST_CHECK_EQUAL(0UL, _ctrl.free());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());

    const std::string datafname = "test_data_file1";
    const std::string datafile = (tmpd / datafname).c_str();
    { std::ofstream _fe; _fe.open(datafile);
     for (int i=0; i<9999; i++) { _fe.write("0123456789",10); }
     _fe.close();
    }
    
    BOOST_REQUIRE( _ctrl.backup(datafile) );

    BOOST_CHECK( _ctrl.bytes_in() > 0);
    BOOST_CHECK( _ctrl.bytes_out() > 0);
    BOOST_CHECK(32*256*1024-100*10000-16 < _ctrl.free());

    _ctrl.finalize();

    auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_backup1.xml";
    auto const fp_dbky = lxr::Options::current().fpathMeta() / "test_dbkey_backup1.xml";
    std::ofstream _out1; _out1.open(fp_dbfp.native());
    _ctrl.getDbFp().outStream(_out1);
    _out1.close();
    std::ofstream _out2; _out2.open(fp_dbky.native());
    _ctrl.getDbKey().outStream(_out2);
    _out2.close();

}
```

## Test case: encrypt a raw file
```cpp
BOOST_AUTO_TEST_CASE( encrypt_file_raw )
{
    auto const tmpd = boost::filesystem::temp_directory_path();
    lxr::Options::set().nChunks(17);
    lxr::Options::set().isCompressed(false);
    lxr::Options::set().fpathChunks() = tmpd / "LXR";
    lxr::Options::set().fpathMeta() = tmpd /"meta";
    lxr::BackupCtrl _ctrl;

    BOOST_CHECK_EQUAL(0UL, _ctrl.free());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());

    const std::string datafname = "test_data_file2";
    const std::string datafile = (tmpd / datafname).c_str();
    { std::ofstream _fe; _fe.open(datafile);
     for (int i=0; i<9999; i++) { _fe.write("0123456789",10); }
     _fe.close();
    }
    
    BOOST_REQUIRE( _ctrl.backup(datafile) );

    BOOST_CHECK( _ctrl.bytes_in() > 0);
    BOOST_CHECK( _ctrl.bytes_out() > 0);
    BOOST_CHECK(17*256*1024-100*10000-16 < _ctrl.free());

    _ctrl.finalize();

    auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_backup2.xml";
    auto const fp_dbky = lxr::Options::current().fpathMeta() / "test_dbkey_backup2.xml";
    std::ofstream _out1; _out1.open(fp_dbfp.native());
    _ctrl.getDbFp().outStream(_out1);
    _out1.close();
    std::ofstream _out2; _out2.open(fp_dbky.native());
    _ctrl.getDbKey().outStream(_out2);
    _out2.close();

}
```

## Test case: encrypt empty file
```cpp
BOOST_AUTO_TEST_CASE( encrypt_empty_file )
{
    auto const tmpd = boost::filesystem::temp_directory_path();
    lxr::Options::set().nChunks(16);
    lxr::Options::set().isCompressed(true);
    lxr::Options::set().fpathChunks() = tmpd / "LXR";
    lxr::Options::set().fpathMeta() = tmpd /"meta";
    lxr::BackupCtrl _ctrl;

    BOOST_CHECK_EQUAL(0UL, _ctrl.free());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_in());
    BOOST_CHECK_EQUAL(0UL, _ctrl.bytes_out());

    const std::string datafile = (tmpd / "test_empty_file").c_str();
    { std::ofstream _fe; _fe.open(datafile); _fe.close(); }
    BOOST_REQUIRE( _ctrl.backup(datafile) );

    BOOST_CHECK( _ctrl.bytes_in() == 0);
    BOOST_CHECK( _ctrl.bytes_out() == 0);
    BOOST_CHECK_EQUAL(16*256*1024-16, _ctrl.free());

    auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_backup3.xml";
    std::ofstream _outs; _outs.open(fp_dbfp.native());
    _ctrl.getDbFp().outStream(_outs);

}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
