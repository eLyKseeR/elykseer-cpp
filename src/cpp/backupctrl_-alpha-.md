```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/backupctrl.hpp"
#include "lxr/assembly.hpp"
#include "lxr/chunk.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/md5.hpp"
#include "lxr/options.hpp"
#include "lxr/sha256.hpp"

#include "sizebounded/sizebounded.hpp"
#include "stream-cpp/zblocks.hpp"
#include "stream-cpp/zblocks.ipp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>

using clk = std::chrono::high_resolution_clock;

namespace lxr {

struct BackupCtrl::pimpl
{
    pimpl() {}
    ~pimpl() {}

    int _nChunks {16};
    std::shared_ptr<Assembly> _ass;
    DbFp _dbfpref;
    DbFp _dbfp;
    DbKey _dbkey;
    uint64_t trx_in {0UL};
    uint64_t trx_out {0UL};
    std::chrono::microseconds time_encr {0};
    std::chrono::microseconds time_extract {0};
    std::chrono::microseconds time_read {0};
    std::chrono::microseconds time_compress {0};
    std::chrono::microseconds time_backup {0};

    private:
};

````
