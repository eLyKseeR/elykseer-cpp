```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/backupctrl.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/os.hpp"
#include "lxr/key256.hpp"
#include "lxr/sha256.hpp"
#include "lxr/restorectrl.hpp"
#include "lxr/options.hpp"

#include "boost/filesystem.hpp"

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

## Test case: backup and restore a file

```cpp
BOOST_AUTO_TEST_CASE( backup_restore_file_raw )
{
  auto const tmpd = boost::filesystem::temp_directory_path();
  lxr::Options::set().fpathChunks() = tmpd / "LXR";
  lxr::Options::set().fpathMeta() = tmpd / "meta";
  if(! lxr::FileCtrl::dirExists(lxr::Options::current().fpathMeta())) {
    boost::filesystem::create_directory(lxr::Options::current().fpathMeta());
  }

  auto const outputdir = tmpd / "restored";
  if(! lxr::FileCtrl::dirExists(outputdir)) {
    boost::filesystem::create_directory(outputdir);
  }
  const std::string datafname = "test_data_file";
  auto const datafile = (tmpd / datafname);
  auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_restore.xml";
  auto const fp_dbky = lxr::Options::current().fpathMeta() / "test_dbkey_restore.xml";

  // will compare SHA256 checksums
  lxr::Key256 hash_0{true}, hash_1{true};

  // cleanup
  {
    auto restoredfp = outputdir / datafile;
    if (boost::filesystem::exists(restoredfp)) {
      boost::filesystem::remove(restoredfp);
    }
  }

  // encrypt file
  {
    lxr::Options::set().isCompressed(false);
    lxr::Options::set().nChunks(16);
    lxr::BackupCtrl _backup;

    { std::ofstream _fe; _fe.open(datafile.native());
     for (int i=0; i<799999; i++) { _fe.write("0123456789",7); }
     _fe.close();
    }
    hash_0 = lxr::Sha256::hash(datafile);

    BOOST_REQUIRE( _backup.backup(datafile) );

    BOOST_CHECK( _backup.bytes_in() > 0);
    BOOST_CHECK( _backup.bytes_out() > 0);
    if (lxr::Options::current().isCompressed()) {
      BOOST_CHECK( _backup.bytes_in() > _backup.bytes_out());  // by compression
    }

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
    lxr::Options::set().nChunks(17);
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
    BOOST_CHECK_EQUAL(2, _dbks.count());
    _restore.addDbKey(_dbks);

    BOOST_REQUIRE( _restore.restore(outputdir, datafile.native()) );
    hash_1 = lxr::Sha256::hash(outputdir / datafile);

    BOOST_CHECK( _restore.bytes_in() > 0);
    BOOST_CHECK( _restore.bytes_out() > 0);
    BOOST_CHECK( _restore.bytes_out() >= _restore.bytes_in());  // by compression
    BOOST_CHECK_EQUAL( hash_0.toHex(), hash_1.toHex() );
    // std::cout << "  sha256 = " << hash_0.toHex() << std::endl;
  }
}
```

## Test case: backup and restore a file using compression

```cpp
BOOST_AUTO_TEST_CASE( backup_restore_file_compressed )
{
  auto const tmpd = boost::filesystem::temp_directory_path();
  lxr::Options::set().fpathChunks() = tmpd / "LXR";
  lxr::Options::set().fpathMeta() = tmpd / "meta";
  if(! lxr::FileCtrl::dirExists(lxr::Options::current().fpathMeta())) {
    boost::filesystem::create_directory(lxr::Options::current().fpathMeta());
  }

  auto const outputdir = tmpd / "restored";
  if(! lxr::FileCtrl::dirExists(outputdir)) {
    boost::filesystem::create_directory(outputdir);
  }
  const std::string datafname = "test_data_file2";
  auto const datafile = (tmpd / datafname);
  auto const fp_dbfp = lxr::Options::current().fpathMeta() / "test_dbfp_restore2.xml";
  auto const fp_dbky = lxr::Options::current().fpathMeta() / "test_dbkey_restore2.xml";

  // will compare SHA256 checksums
  lxr::Key256 hash_0{true}, hash_1{true};
  int expected_blocks = 2;

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

    { std::ofstream _fe; _fe.open(datafile.native());
      for (int i=0; i<14; i++) {
        std::ifstream _fi;
#if defined(__APPLE__)
         _fi.open("/bin/bash", std::ifstream::in | std::ifstream::binary);
         expected_blocks = 3;
#elif defined(__linux__)
         _fi.open("/bin/sh");
#elif defined(_WIN32)
         _fi.open("C:\\Windows\\notepad.exe");
#else
         _fi.open("/bin/sh");
#endif
        char buffer[1025];
        int nread = 0;
        while (_fi.good()) {
          _fi.read(buffer, 1024);
          nread = _fi.gcount();
          _fe.write(buffer, nread);
        }
        _fi.close();
      }
      _fe.close();
    }
    hash_0 = lxr::Sha256::hash(datafile);

    BOOST_REQUIRE( _backup.backup(datafile) );

    BOOST_CHECK( _backup.bytes_in() > 0);
    BOOST_CHECK( _backup.bytes_out() > 0);
    if (lxr::Options::current().isCompressed()) {
      BOOST_CHECK( _backup.bytes_in() > _backup.bytes_out());  // by compression
    }
    std::clog << "bytes in: " << _backup.bytes_in() << " out: " << _backup.bytes_out() << std::endl;
    std::clog << "encryption time: " << _backup.time_encrypt() << std::endl;
    std::clog << "extraction time: " << _backup.time_extract() << std::endl;
    std::clog << "write time: " << _backup.time_write() << std::endl;

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
    lxr::Options::set().nChunks(17);
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
    BOOST_CHECK_EQUAL(expected_blocks, _dbks.count());
    _restore.addDbKey(_dbks);

    BOOST_REQUIRE( _restore.restore(outputdir, datafile.native()) );
    hash_1 = lxr::Sha256::hash(outputdir / datafile);

    BOOST_CHECK( _restore.bytes_in() > 0);
    BOOST_CHECK( _restore.bytes_out() > 0);
    BOOST_CHECK( _restore.bytes_out() > _restore.bytes_in());  // by compression
    BOOST_CHECK_EQUAL( hash_0.toHex(), hash_1.toHex() );
    // std::cout << "  sha256 = " << hash_0.toHex() << std::endl;
    std::clog << "bytes in: " << _restore.bytes_in() << " out: " << _restore.bytes_out() << std::endl;
    std::clog << "decryption time: " << _restore.time_decrypt() << std::endl;
    std::clog << "decompression time: " << _restore.time_decompress() << std::endl;
    std::clog << "read time: " << _restore.time_read() << std::endl;
    std::clog << "write time: " << _restore.time_write() << std::endl;
  }
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
