```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "lxr/dbfp.hpp"
#include "lxr/dbkey.hpp"
#include "boost/filesystem.hpp"

#include <chrono>
#include <ctime>
#include <memory>

````

namespace [lxr](namespace.list) {

/*

```fsharp
module BackupCtrl =

    exception BadAccess of string
    exception ReadFailed
    exception WriteFailed

    type t

    val create : Options -> t

    val setReference : t -> DbKey option -> DbFp option -> unit

    val finalize : t -> unit

    val backup : t -> string -> unit

    val free : t -> int

    val bytes_in : t -> int64
    val bytes_out : t -> int64

    val time_encrypt : t -> int
    val time_extract : t -> int
    val time_write : t -> int

    val getDbKeys : t -> DbKey
    val getDbFp : t -> DbFp
```

*/

# class BackupCtrl

{

>public:

>explicit [BackupCtrl](backupctrl_ctor.cpp.md)();

>[~BackupCtrl](backupctrl_ctor.cpp.md)();

>void [setReference](backupctrl_functions.cpp.md)();

>void [finalize](backupctrl_functions.cpp.md)();

>bool [backup](backupctrl_functions.cpp.md)(boost::filesystem::path const &);

>bool [backup](backupctrl_functions.cpp.md)(std::istream const & stream, std::string const & name);

>uint32_t [free](backupctrl_info.cpp.md)() const;

>uint64_t [bytes_in](backupctrl_info.cpp.md)() const;

>uint64_t [bytes_out](backupctrl_info.cpp.md)() const;

>std::chrono::microseconds [time_encrypt](backupctrl_info.cpp.md)() const;

>std::chrono::microseconds [time_extract](backupctrl_info.cpp.md)() const;

>std::chrono::microseconds [time_read](backupctrl_info.cpp.md)() const;

>std::chrono::microseconds [time_compression](backupctrl_info.cpp.md)() const;

>std::chrono::microseconds [time](backupctrl_info.cpp.md)() const;

>DbFp const & [getDbFp](backupctrl_info.cpp.md)() const;

>DbKey const & [getDbKey](backupctrl_info.cpp.md)() const;

>protected:

>private:

>struct pimpl;

>std::unique_ptr&lt;pimpl&gt; _pimpl;

>BackupCtrl(BackupCtrl const &) = delete;

>BackupCtrl & operator=(BackupCtrl const &) = delete;

};

```cpp
} // namespace
```
