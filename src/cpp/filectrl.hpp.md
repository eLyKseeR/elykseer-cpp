```cpp

/*
<fpaste ../../../src/copyright.md>
*/

#pragma once

#include <string>

#include <filesystem>
typedef std::filesystem::path filepath;

#include <cstdint>
#include <ctime>
#include <optional>
#include <vector>
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

>static std::optional&lt;std::filesystem::file_time_type&gt; [fileLastWriteTime](filectrl_functions.cpp.md)(filepath const &) noexcept;

>static std::optional&lt;uint64_t&gt; [fileSize](filectrl_functions.cpp.md)(filepath const &) noexcept;

>static bool [fileExists](filectrl_functions.cpp.md)(filepath const &) noexcept;

>static bool [isFileReadable](filectrl_functions.cpp.md)(filepath const &) noexcept;

>static bool [dirExists](filectrl_functions.cpp.md)(filepath const &) noexcept;

>static std::vector&lt;filepath&gt; [fileListRecursive](filectrl_functions.cpp.md)(filepath const &);

>protected:

>FileCtrl() {}

>private:

>FileCtrl(FileCtrl const &) = delete;

>FileCtrl & operator=(FileCtrl const &) = delete;

};

```cpp
} // namespace

```