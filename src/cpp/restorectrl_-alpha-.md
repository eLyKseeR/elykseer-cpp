```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/restorectrl.hpp"
#include "lxr/assembly.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/key256.hpp"
#include "lxr/md5.hpp"
#include "lxr/options.hpp"
#include "lxr/sha256.hpp"

#include "sizebounded/sizebounded.hpp"
#include "stream-cpp/zblocks.hpp"
#include "stream-cpp/zblocks.ipp"

#include <iostream>
#include <chrono>
#include <fstream>

namespace lxr {

struct RestoreCtrl::pimpl
{
    pimpl() {}
    ~pimpl() {}

    std::shared_ptr<Assembly> _ass;
    DbFp _dbfp;
    DbKey _dbkey;
    uint64_t trx_in {0UL};
    uint64_t trx_out {0UL};
    std::chrono::microseconds time_read {0};
    std::chrono::microseconds time_decr {0};
    std::chrono::microseconds time_write {0};
    
    private:
};

````
