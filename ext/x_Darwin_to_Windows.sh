#!/bin/sh

MINGW_PATH="/opt/homebrew/Cellar/mingw-w64/11.0.1/toolchain-x86_64"
if [ ! -d ${MINGW_PATH} ]; then
    echo "missing path to mingw toolchain"
    exit 1
fi

CC=${MINGW_PATH}/bin/x86_64-w64-mingw32-gcc
if [ ! -e $CC ]; then
    echo "missing cross compiler in $CC"
    exit 1
fi
export CC

CXX=${MINGW_PATH}/bin/x86_64-w64-mingw32-g++
if [ ! -e $CXX ]; then
    echo "missing cross compiler in $CXX"
    exit 1
fi
export CXX

AR=${MINGW_PATH}/bin/x86_64-w64-mingw32-ar
if [ ! -e $AR ]; then
    echo "missing command ar in $AR"
    exit 1
fi
export AR

RANLIB=${MINGW_PATH}/bin/x86_64-w64-mingw32-ranlib
if [ ! -e $RANLIB ]; then
    echo "missing command ranlib in $RANLIB"
    exit 1
fi
export RANLIB

make xWin64 
