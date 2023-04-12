#!/bin/sh

if [[ $(uname -s) != MINGW64* ]]; then
	echo "this compilation is only valid on a MinGW64 installation"
	exit 1
fi

# cleanup
rm -rf CMakeCache.txt CMakeFiles src/CMakeFiles src/cli/CMakeFiles test/CMakeFiles

set -e

export CC=gcc
export CXX=g++

# where the compiled dependencies have been installed
export PKG_CONFIG_PATH=$(pwd)/../ext/win64/lib/pkgconfig:$PKG_CONFIG_PATH

# compile in Debug | Release mode
#MODE=Debug
MODE=Release
cmake --fresh -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${MODE} .
cmake --build .

# show output
ls -l src/libelykseer-cpp*.a

