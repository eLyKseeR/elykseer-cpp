```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

#include "boost/filesystem.hpp"
#include <string>
#include <vector>
#include <ctime>
#include <optional>
````

namespace [lxr](namespace.list) {

/*

```fsharp
module FileCtrl = 

    type FilePath = string

    val fileDate : FilePath -> string
    val fileLastWriteTime : FilePath -> System.DateTime

    val fileSize : FilePath -> int64

    val fileExists : FilePath -> bool

    val dirExists : FilePath -> bool

    val isFileReadable : FilePath -> bool

    val fileListRecursive : FilePath -> FilePath list
```

*/

# class FileCtrl

{

>public:

>static std::optional&lt;std::string&gt; [fileDate](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static std::optional&lt;std::time_t&gt; [fileLastWriteTime](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static std::optional&lt;uint64_t&gt; [fileSize](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static bool [fileExists](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static bool [isFileReadable](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static bool [dirExists](filectrl_functions.cpp.md)(boost::filesystem::path const &) noexcept;

>static std::vector&lt;boost::filesystem::path&gt; [fileListRecursive](filectrl_functions.cpp.md)(boost::filesystem::path const &);

>protected:

>FileCtrl() {}

>private:

>FileCtrl(FileCtrl const &) = delete;

>FileCtrl & operator=(FileCtrl const &) = delete;

};

```cpp
} // namespace

inline boost::filesystem::path operator+(boost::filesystem::path leftp, boost::filesystem::path rightp) { return boost::filesystem::path(leftp) += rightp; }

```