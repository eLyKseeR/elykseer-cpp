# eLyKseeR

![eLyKseeR](https://www.elykseer.com/wp-content/uploads/2017/11/lxricon2.png)

cryptographic data archive; written in C++; an elixir to stay forever

read [more](https://github.com/eLyKseeR/elykseer-cpp)

[![Compilation of project with submodules and verification by unit tests](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yml/badge.svg?branch=main)](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yml)

# building

1. code extraction and compilation (in directory `build`)

> $ cd build

> $ cmake .

> $ make

2. testing (in directory `build`)

> $ ./ut.sh


# cross compilation

cd build
cmake --fresh -DCMAKE_TOOLCHAIN_FILE=Toolchain_Darwin_to_Windows.cmake --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Release .
make && make install/strip

# copyright

Copyright 2018-2023 by Alexander Diemand

# license

This work is licensed under the 
[GNU General Public License v3](https://www.gnu.org/licenses/gpl.html)

