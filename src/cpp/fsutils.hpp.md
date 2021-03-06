```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <utility>

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

>static std::string [sep](fsutils_functions.cpp.md)();

>static const boost::filesystem::path [cleanfp](fsutils_functions.cpp.md)(boost::filesystem::path const &);

>static std::pair&lt;const std::string, const std::string&gt; [osusrgrp](fsutils_functions.cpp.md)(boost::filesystem::path const &);

>static const std::string [fstem](fsutils_functions.cpp.md)();

>protected:

>FsUtils() {}

>private:

>FsUtils(FsUtils const &) = delete;

>FsUtils & operator=(FsUtils const &) = delete;

};


>const boost::filesystem::path [operator/](fsutils_functions.cpp.md)(std::string const &a, std::string const &b);

>const boost::filesystem::path [operator/](fsutils_functions.cpp.md)(boost::filesystem::path const &a, std::string const &b);

>const boost::filesystem::path [operator/](fsutils_functions.cpp.md)(boost::filesystem::path const &a, boost::filesystem::path const &b);


```cpp
} // namespace
```
