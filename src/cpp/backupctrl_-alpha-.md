```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/backupctrl.hpp"
#include "lxr/assembly.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/dbfp.hpp"
#include "lxr/dbkey.hpp"
#include "lxr/md5.hpp"
#include "lxr/sha256.hpp"

#include "sizebounded/sizebounded.hpp"
#include "stream-cpp/zblocks.hpp"
#include "stream-cpp/zblocks.ipp"

#include <iostream>
#include <cstdio>

namespace lxr {

struct BackupCtrl::pimpl
{
    pimpl(Options const & o)
        : _o(o)
    {
        _ass.reset(new Assembly(o.nChunks()));
    }
    ~pimpl() {}

    bool renew_assembly();

    Options _o;
    std::shared_ptr<Assembly> _ass;
    DbFp _dbfp;
    DbKey _dbkey;
    DbFp _reffp;
    uint64_t trx_in {0UL};
    uint64_t trx_out {0UL};
    std::chrono::microseconds time_encr {0};
    std::chrono::microseconds time_decr {0};
    std::chrono::microseconds time_write {0};

    private:
    pimpl();
};

````
