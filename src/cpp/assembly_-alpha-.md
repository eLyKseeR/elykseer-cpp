```cpp
/*
```
<fpaste ../../src/copyright.md>
```cpp
*/
```

```cpp
#include "lxr/assembly.hpp"
#include "lxr/chunk.hpp"
#include "lxr/appid.hpp"
#include "lxr/sha256.hpp"
#include "lxr/aes.hpp"
#include "lxr/options.hpp"
#include "lxr/filectrl.hpp"
#include "lxr/fsutils.hpp"

#include "sizebounded/sizebounded.hpp"
#include "sizebounded/sizebounded.ipp"

#include <iostream>
#include <string>
#ifdef OpenMP_available
#include <omp.h>
#endif

namespace lxr {

enum tAstate { readable=1, writable=2, encrypted=4 };

struct Assembly::pimpl
{
  pimpl(Key256 const & aid, Nchunks const & n) :
      _chunks(new Chunk[n.nchunks()])
    , _n(n)
    , _aid(aid)
  { }

  explicit pimpl(Nchunks const & n)
    : pimpl(mk_aid(), n)
  { }

  ~pimpl() {
    if (_chunks) {
      delete[] _chunks; }
  }

  std::string said() const { return _aid.toHex(); }

  Key256 mk_aid() const {
    Key256 k;
    std::string b(AppId::appid());
    return Sha256::hash(b.append(k.toHex()));
  }

  int set_data(const int pos, const int dlen, const unsigned char *d, int p0);
  int get_data(const int pos, const int dlen, unsigned char *d) const;

  Chunk *_chunks{nullptr};
  Nchunks _n;
  Key256  _aid;
  int _pos {0};
#ifdef DEBUG
  // the data is readable even if not encrypted
  int _state {writable | readable};
#else
  int _state {writable};
#endif

private: 
pimpl()=delete;
};

````
