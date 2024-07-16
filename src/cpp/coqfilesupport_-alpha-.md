```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/coqfilesupport.hpp"
#include "lxr/os.hpp"
#include "lxr/sha3.hpp"

#include <filesystem>
#include <sstream>

#if !defined(PLATFORM_win64)
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace lxr {

````
