```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#pragma once

#include <string>
#include <utility>
#include <optional>

#include "lxr/filectrl.hpp"

````

namespace [lxr](namespace.list) {

/*

```fsharp

module FsUtils =

    //val eol : string

    val sep : string

    val cleanfp : string -> string

    val osusrgrp : string -> string * string

    val fstem : unit -> string
```

*/

# class FsUtils

{

>public:

>static std::string [sep](fsutils_functions.cpp.md)() noexcept;

>static const filepath [cleanfp](fsutils_functions.cpp.md)(filepath const &) noexcept;

>static std::pair&lt;const std::string, const std::string&gt; [osusrgrp](fsutils_functions.cpp.md)(filepath const &);

>static const std::string [fstem](fsutils_functions.cpp.md)() noexcept;

>protected:

>FsUtils() {}

>private:

>FsUtils(FsUtils const &) = delete;

>FsUtils & operator=(FsUtils const &) = delete;

};


>const filepath [operator/](fsutils_functions.cpp.md)(std::string const &a, std::string const &b) noexcept;

>const filepath [operator/](fsutils_functions.cpp.md)(filepath const &a, std::string const &b) noexcept;

>const filepath [operator/](fsutils_functions.cpp.md)(filepath const &a, filepath const &b) noexcept;


```cpp
} // namespace
```
