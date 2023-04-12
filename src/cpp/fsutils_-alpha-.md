```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/fsutils.hpp"
#include "lxr/os.hpp"

#include <iostream>

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#else
#include <windows.h>
#include "aclapi.h"
#pragma comment(lib, "advapi32.lib")
#endif

namespace lxr {

````
