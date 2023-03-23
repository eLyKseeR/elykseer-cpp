```cpp

/*
<fpaste ../../src/copyright.md>
*/

#pragma once

````

namespace [lxr](namespace.list) {


# class Nchunks

{

>public:

>static constexpr int min_n { 16 };

>static constexpr int max_n { 256 };

>explicit [Nchunks](nchunks_ctor.cpp.md)(int n);

>Nchunks(Nchunks const &) = default;

>Nchunks & operator=(Nchunks const &) = default;

>~Nchunks() = default;

>int [nchunks](nchunks_functions.cpp.md)() const;

>private:

>Nchunks();

>int _n {min_n};

};

```cpp
} // namespace
```
