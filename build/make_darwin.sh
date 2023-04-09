#!/bin/sh
#
# need to install the GNU version of binutils (i.e. ar):
#
# brew install binutils

if [[ $(uname -s) != Darwin ]]; then
	echo "this compilation is only valid on a Mac installation"
	exit 1
fi

# cleanup
rm -rf CMakeCache.txt CMakeFiles src/CMakeFiles test/CMakeFiles

set -e

# where the compiled dependencies have been installed
#export PKG_CONFIG_PATH=$(pwd)/../ext/darwin/lib/pkgconfig:$PKG_CONFIG_PATH

# compile in Debug | Release mode
#MODE=Debug
MODE=Release
cmake --fresh -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${MODE} .
cmake --build .

# show output
ls -l src/libelykseer-cpp*.a

