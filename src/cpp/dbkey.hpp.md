```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "lxr/dbctrl.hpp"
#include "lxr/key128.hpp"
#include "lxr/key256.hpp"
#include <string>
````

namespace [lxr](namespace.list) {

## struct DbKeyBlock
```c++
{
    DbKeyBlock() : _n(16) {};
    Key256 _key;
    Key128 _iv;
    int _n;
};
std::ostream & operator<<(std::ostream &os, lxr::DbKeyBlock const & block);
```

# class DbKey : public DbCtrl&lt;struct DbKeyBlock&gt;

{

>public:

>DbKey() : DbCtrl&lt;struct DbKeyBlock&gt;() {}

>virtual void [inStream](dbkey_functions.cpp.md)(std::istream &) override;

>virtual void [outStream](dbkey_functions.cpp.md)(std::ostream &) const override;

>protected:

>private:

>DbKey(DbKey const &) = delete;

>DbKey & operator=(DbKey const &) = delete;

};

```cpp
} // namespace
```
