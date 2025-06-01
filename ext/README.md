External Dependencies
=====================

the content of the 'ext/' directory:

  elykseer-crypto/
  date/
  gitalk/
  os-metrics/
  rapidcheck/
  sizebounded/
  stream-cpp/


elykseer-crypto
===============

license: GPL-3.0

https://github.com/eLyKseeR/elykseer-crypto


gitalk
======

license: BSD3

https://github.com/CodiePP/gitalk.git


sizebounded
===========

license: BSD3

https://github.com/CodiePP/sizebounded.git


stream-cpp
==========

license: BSD3

https://github.com/CodiePP/stream-cpp.git


date
====

license: MIT

https://github.com/HowardHinnant/date.git


RapidCheck
==========

QuickCheck clone for C++

license: BSD2

https://github.com/emil-e/rapidcheck


OS-metrics
==========

Access operating system counters for memory and CPU usage

license: GPL-3.0

https://github.com/CodiePP/os-metrics.git


## cross compilation

### for Windows


Prepare dependencies in "ext":

1. cd ext

2. download a zip and extract here: https://github.com/skeeto/w64devkit/releases

3. set environment variables CC, CXX, AR, RANLIB to point to "x86_64-w64-mingw32-gcc"

4. run `make xWin64`


Manually compile more dependencies:

1. boost: version 1.83.0 

   1.1 create a 'user-config.jam' in `boost_1_83_0/tools/build/src/contrib/user-config.jam`

```
using gcc : 14.2.0 : "/opt/homebrew/Cellar/mingw-w64/12.0.0_2/toolchain-x86_64/bin/x86_64-w64-mingw32-g++" :
   <archiver>/opt/homebrew/Cellar/mingw-w64/12.0.0_2/toolchain-x86_64/bin/x86_64-w64-mingw32-ar
   <ranlib>/opt/homebrew/Cellar/mingw-w64/12.0.0_2/toolchain-x86_64/bin/x86_64-w64-mingw32-ranlib
   <rc>/opt/homebrew/Cellar/mingw-w64/12.0.0_2/toolchain-x86_64/bin/x86_64-w64-mingw32-windres
   <as>/opt/homebrew/Cellar/mingw-w64/12.0.0_2/toolchain-x86_64/bin/x86_64-w64-mingw32-as
;
```

   1.2 compile and install
   `./b2 --prefix=$(pwd)/../xWindows_x86_64 --debug-configuration link=static toolset=gcc-14.2.0 target-os=windows address-model=64 --without-mpi --without-python variant=release install`

   first: `./bootstrap.sh --prefix=$(pwd)/../xWindows_x86_64 link=static toolset=gcc-14.2.0 target-os=windows address-model=64 --without-libraries=mpi,python variant=release install`

2. zlib: https://zlib.net/zlib-1.3.tar.xz

   2.1 mkdir build; cd build

   2.2 cmake --fresh -DCMAKE_TOOLCHAIN_FILE=../../../support/Toolchain_Darwin_to_Windows.cmake --install-prefix=$(pwd)/../../xWindows_x86_64 -DCMAKE_BUILD_TYPE=Release ..

   2.3 make & make install


Compile project using platform file:

1. mkdir -p dist; cd build

2. cmake --fresh -DCMAKE_TOOLCHAIN_FILE=Toolchain_Darwin_to_Windows.cmake --install-prefix=$(pwd)/../dist -DCMAKE_BUILD_TYPE=Release .

3. make && make install/strip

