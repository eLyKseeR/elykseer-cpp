```cpp
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif

#include "boost/test/unit_test.hpp"

#include "lxr/fsutils.hpp"
#include "lxr/options.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
````

# Test suite: utOptions

on class [Options](../src/options.hpp.md)

```cpp
BOOST_AUTO_TEST_SUITE( utOptions )
```
## Test case: export to and import from XML
```cpp
BOOST_AUTO_TEST_CASE( export_import_XML )
{
    lxr::Options & _opts = lxr::Options::set();
    _opts.nChunks(17);
    _opts.fpathChunks() = "/data/storage";
    _opts.fpathMeta() = "/mnt/secure";
    auto const tmpd = boost::filesystem::temp_directory_path();
	auto const _fpath = tmpd / "test_options_1.xml";
	std::ofstream _outs; _outs.open(_fpath.native());
    _opts.outStream(_outs);
    _outs.close();
    lxr::Options & _opts1 = lxr::Options::set();
	std::ifstream _ins; _ins.open(_fpath.native());
    _opts1.inStream(_ins);
    auto const _opts2 = lxr::Options::current();
    BOOST_CHECK_EQUAL(17, _opts2.nChunks());
    BOOST_CHECK_EQUAL("/data/storage", _opts2.fpathChunks());
    BOOST_CHECK_EQUAL("/mnt/secure", _opts2.fpathMeta());
}
```

## Test case: set and get parameters
```cpp
BOOST_AUTO_TEST_CASE( setters_getters )
{
    lxr::Options & _opts = lxr::Options::set();
    _opts.nChunks(17);
    _opts.fpathChunks() = "/data/storage";
    _opts.fpathMeta() = "/mnt/secure";

    auto _opts2 = lxr::Options::current();
    BOOST_CHECK_EQUAL(17, _opts2.nChunks());
    BOOST_CHECK_EQUAL("/data/storage", _opts2.fpathChunks());
    BOOST_CHECK_EQUAL("/mnt/secure", _opts2.fpathMeta());

    std::string _buf;
    std::ostringstream _outs;
    _opts.outStream(_outs); _outs.flush();
    _buf = _outs.str();

    std::string goldxml =
        "<Options>\\n"
        "  <memory nchunks=\\"17\\" redundancy=\\"0\\" />\\n"
        "  <compression>on</compression>\\n"
        "  <deduplication level=\\"2\\" />\\n"
        "  <fpaths>\\n"
        "    <meta>/mnt/secure</meta>\\n"
        "    <chunks>/data/storage</chunks>\\n"
        "  </fpaths>\\n"
        "</Options>\\n";
    BOOST_CHECK_EQUAL(goldxml, _buf);
}
```

```xml
<Options>
  <memory nchunks="17" redundancy="0" />
  <compression>on</compression>
  <deduplication level="2" />
  <fpaths>
    <meta>/mnt/secure</meta>
    <chunks>/data/storage</chunks>
  </fpaths>
</Options>
```

## Test case: read from XML
```cpp
BOOST_AUTO_TEST_CASE( readXML )
{
    std::string goldxml =
        "<Options>"
        "  <memory nchunks=\\"17\\" redundancy=\\"0\\" />"
        "  <compression>on</compression>"
        "  <deduplication level=\\"2\\" />"
        "  <fpaths>"
        "    <meta>/mnt/secure</meta>"
        "    <chunks>/data/storage</chunks>"
        "  </fpaths>"
        "</Options>";
    lxr::Options & _opts = lxr::Options::set();
    _opts.nChunks(27);
    std::istringstream _ins; _ins.str(goldxml);
    _opts.inStream(_ins);

    BOOST_CHECK_EQUAL(17, _opts.nChunks());
    BOOST_CHECK_EQUAL(0, _opts.nRedundancy());
    BOOST_CHECK_EQUAL(true, _opts.isCompressed());
    BOOST_CHECK_EQUAL(2, _opts.isDeduplicated());
    BOOST_CHECK_EQUAL("/data/storage", _opts.fpathChunks());
    BOOST_CHECK_EQUAL("/mnt/secure", _opts.fpathMeta());
}
```

```cpp
BOOST_AUTO_TEST_SUITE_END()
```
