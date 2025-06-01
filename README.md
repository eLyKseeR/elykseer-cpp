# eLyKseeR

![eLyKseeR](https://www.elykseer.com/wp-content/uploads/2017/11/lxricon2.png)

cryptographic data archive; written in C++; an elixir to stay forever

read [more](https://github.com/eLyKseeR/elykseer-cpp)

[![Compilation of project with submodules and verification by unit tests](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yml/badge.svg?branch=main)](https://github.com/eLyKseeR/elykseer-cpp/actions/workflows/CI.yml)

# building

1. code extraction and compilation (in directory `build`)

> $ mkdir -vp build; cd build

> $ cmake -GNinja --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Debug ..

> $ ninja

2. testing (in directory `build`)

> $ ./tests/cpp/utelykseer-cpp


# cross compilation

- build Docker image 'xcompile'
   - `docker build -t xcompile .` 

- launch Docker image 'xcompile'
   - ``docker run -it --rm --user=$(id -u):$(id -g) -v .:/work xcompile``
```sh
export PATH=${HOME}/.local/bin:${PATH}
cd build
cmake --fresh -DCMAKE_TOOLCHAIN_FILE=../support/Toolchain_Darwin_to_Windows.cmake --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Release -GNinja ..
ninja && ninja install/strip
```

# copyright

Copyright 2018-2025 by Alexander Diemand

# license

This work is licensed under the 
[GNU General Public License v3](https://www.gnu.org/licenses/gpl.html)

