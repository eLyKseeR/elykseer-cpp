```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/dbbackupjob.hpp"
#include "lxr/fsutils.hpp"

#include <iostream>
#include <fstream>
````

# Test suite: utDbBackupJob

on class [DbBackupJob](../src/dbbackupjob.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utDbBackupJob )
```
## Test case: set and get record
```cpp
BOOST_AUTO_TEST_CASE( set_get_record )
{
  const std::string name1 = "all my precious data";
  const std::string name2 = "the ledger";

  lxr::DbBackupJob _db;
  lxr::DbJobDat _job1;
  _job1._paths.push_back(std::make_pair("file","/home/me/Data/Performance.ods"));
  _job1._paths.push_back(std::make_pair("file","/home/me/Data/Base_Input.csv"));
  _job1._regexincl.push_back(std::regex(".*"));
  lxr::DbJobDat _job2;
  _job2._paths.push_back(std::make_pair("file","/home/me/Blocks.dat"));
  _job2._regexincl.push_back(std::regex(".*"));
  _db.set(name1, _job1);
  _db.set(name2, _job2);
  auto ob1 = _db.get(name1);
  auto ob2 = _db.get(name2);
  BOOST_CHECK(ob1);
  BOOST_CHECK(ob2);
  BOOST_CHECK_EQUAL(2, _db.count());
  BOOST_CHECK_EQUAL(1, ob2->_regexincl.size());
  BOOST_CHECK_EQUAL(0, ob1->_regexexcl.size());
  BOOST_CHECK_EQUAL(1, ob2->_paths.size());
  BOOST_CHECK_EQUAL(2, ob1->_paths.size());
}
```

```xml
<?xml version="1.0"?>
<DbBackupJob xmlns="http://spec.sbclab.com/lxr/v1.0">
<library><name>LXR</name><version>Version 1.5.1.1 TR5 - do not use for production</version></library>
<host>AlexBook.fritz.box</host>
<user>root</user>
<date>20201219T091914</date>
  <Job name="the ledger">
<Options>
  <memory nchunks="16" redundancy="0" />
  <compression>on</compression>
  <deduplication level="2" />
  <fpaths>
    <meta>/var/folders/kn/cplf9ykx42v2g07g98zjqgs40000zq/T/</meta>
    <chunks>/var/folders/kn/cplf9ykx42v2g07g98zjqgs40000zq/T/</chunks>
  </fpaths>
</Options>
    <Paths>
    <path type="file">/home/me/Blocks.dat</path>
    </Paths>
    <Filters>
    </Filters>
  </Job>
  <Job name="all my precious data">
<Options>
  <memory nchunks="16" redundancy="0" />
  <compression>on</compression>
  <deduplication level="2" />
  <fpaths>
    <meta>/var/folders/kn/cplf9ykx42v2g07g98zjqgs40000zq/T/</meta>
    <chunks>/var/folders/kn/cplf9ykx42v2g07g98zjqgs40000zq/T/</chunks>
  </fpaths>
</Options>
    <Paths>
    <path type="file">/home/me/Data/Performance.ods</path>
    <path type="file">/home/me/Data/Base_Input.csv</path>
    </Paths>
    <Filters>
    </Filters>
  </Job>
</DbBackupJob>
```

## Test case: output to XML file
```cpp
BOOST_AUTO_TEST_CASE( output_to_xml )
{
  const std::string name1 = "all my precious data";
	const std::string name2 = "the ledger";
	
  auto const tmpd = std::filesystem::temp_directory_path();
      
	lxr::DbBackupJob _db;
    lxr::DbJobDat _job1;
    _job1._paths.push_back(std::make_pair("file","/home/me/Data/Performance.ods"));
    _job1._paths.push_back(std::make_pair("file","/home/me/Data/Base_Input.csv"));
    _job1._regexincl.push_back(std::regex(".*"));
	lxr::DbJobDat _job2;
    _job2._paths.push_back(std::make_pair("file","/home/me/Blocks.dat"));
    _job2._regexincl.push_back(std::regex(".*"));
	_db.set(name1, _job1);
	_db.set(name2, _job2);
	BOOST_CHECK_EQUAL(2, _db.count());
  auto const fp_db = tmpd / "test_dbbackupjob_1.xml";
	std::ofstream _outs; _outs.open(fp_db.string());
	_db.outStream(_outs);
}
```

## Test case: input from XML file
```cpp
BOOST_AUTO_TEST_CASE( input_from_xml )
{
  const std::string name1 = "all my precious data";
  const std::string name2 = "the ledger";

  auto const tmpd = std::filesystem::temp_directory_path();

  lxr::DbBackupJob _db;
  auto const fp_db = tmpd / "test_dbbackupjob_1.xml";
  std::ifstream _ins; _ins.open(fp_db.string());
  _db.inStream(_ins);
  BOOST_CHECK_EQUAL(2, _db.count());
  BOOST_CHECK(_db.get(name1));
  BOOST_CHECK(_db.get(name2));
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
