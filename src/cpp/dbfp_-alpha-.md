```cpp
/*
```
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/dbfp.hpp"
#include "lxr/liz.hpp"
#include "lxr/os.hpp"
#include "lxr/sha256.hpp"
#include "lxr/filectrl.hpp"

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include "boost/chrono.hpp"
using clk = boost::chrono::high_resolution_clock;

#include <iostream>
#include <functional>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/stat.h>
#endif

namespace lxr {

```
