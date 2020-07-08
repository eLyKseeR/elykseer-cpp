```cpp
/*
````
<fpaste ../../src/copyright.md>
```cpp
*/

#include "lxr/restorectrl.hpp"
#include "lxr/assembly.hpp"
#include "lxr/chunk.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"
#include "lxr/key.hpp"
#include "lxr/key128.hpp"
#include "lxr/key256.hpp"
#include "lxr/md5.hpp"
#include "lxr/options.hpp"
#include "lxr/sha256.hpp"

#include "sizebounded/sizebounded.hpp"
#include "stream-cpp/zblocks.hpp"
#include "stream-cpp/zblocks.ipp"

#include "boost/filesystem.hpp"

#include <iostream>
#include <chrono>
#include <fstream>

namespace lxr {

struct RestoreCtrl::pimpl
{
    pimpl() {}
    ~pimpl() {
        _ass.reset();
      }

    bool load_assembly(Key256 const &);

    template <typename Ct, typename St, typename Vt, int sz>
    int restore_block( DbFpBlock const &block
                     , sizebounded<Vt, sz> &buffer
                     , St &st
                     , std::ofstream &fout
                     , inflatestream<Ct,St,Vt,sz> &decomp );

    std::shared_ptr<Assembly> _ass=nullptr;
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
