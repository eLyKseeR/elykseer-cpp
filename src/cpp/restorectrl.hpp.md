```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#pragma once

#include "lxr/filectrl.hpp"
#include "lxr/dbfp.hpp"
#include "lxr/dbkey.hpp"

#include <chrono>
#include <ctime>
#include <memory>

````

namespace [lxr](namespace.list) {

/*

```fsharp
module RestoreCtrl =

    exception BadAccess
    exception ReadFailed of string
    exception NoKey

    type t

    val create : unit -> t
    val setOptions : t -> Options -> unit

    type FilePath = string
    type RootDir = string

    val restore : t -> RootDir -> FilePath -> unit

    val bytes_in : t -> int64
    val bytes_out : t -> int64

    val time_decrypt : t -> int
    val time_extract : t -> int
    val time_read : t -> int

    val getDbKeys : t -> DbKey
    val getDbFp : t -> DbFp
```

*/

# class RestoreCtrl

{

>public:

>explicit [RestoreCtrl](restorectrl_ctor.cpp.md)();

>[~RestoreCtrl](restorectrl_ctor.cpp.md)();

## // provide meta data

>void [addDbFp](restorectrl_functions.cpp.md)(DbFp const &);

>void [addDbKey](restorectrl_functions.cpp.md)(DbKey const &);

## // restore a file (defined by _fp_) and place in directory _root_

>bool [restore](restorectrl_functions.cpp.md)(filepath const & root, std::string const & fp);

## // restore a file (defined by _fp_) to a stream

>bool [restore](restorectrl_functions.cpp.md)(std::ostream & stream, std::string const & fp);

## // verify a file

>bool [verify](restorectrl_functions.cpp.md)(std::string const & fp);

## // access counters

>uint64_t [bytes_in](restorectrl_info.cpp.md)() const;

>uint64_t [bytes_out](restorectrl_info.cpp.md)() const;

>std::chrono::microseconds [time_decrypt](restorectrl_info.cpp.md)() const;

>std::chrono::microseconds [time_decompress](restorectrl_info.cpp.md)() const;

>std::chrono::microseconds [time_read](restorectrl_info.cpp.md)() const;

>std::chrono::microseconds [time_write](restorectrl_info.cpp.md)() const;

>std::chrono::microseconds [time](restorectrl_info.cpp.md)() const;

>protected:

>private:

>struct pimpl;

>std::unique_ptr&lt;pimpl&gt; _pimpl;

>RestoreCtrl(RestoreCtrl const &) = delete;

>RestoreCtrl & operator=(RestoreCtrl const &) = delete;

};

```cpp
} // namespace
```
