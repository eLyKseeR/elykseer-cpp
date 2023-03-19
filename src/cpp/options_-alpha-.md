```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/options.hpp"

#include "boost/property_tree/xml_parser.hpp"

#include <filesystem>
#include <iostream>

namespace lxr {

struct Options::pimpl {
  pimpl() {
    auto tmpd = std::filesystem::temp_directory_path();
    _fpathmeta = tmpd;
    _fpathchunks =  tmpd;
  }
  int _nchunks{16};
  int _nredundancy {0};
  bool _iscompressed {true};
  int _isdeduplicated {2};
  filepath _fpathchunks;
  filepath _fpathmeta;
};


````
