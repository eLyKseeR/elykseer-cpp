#!/bin/bash

GITALK=`pwd`/../ext/gitalk

cd src
cd lxr
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/elykseer-cpp.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
done

cd ..
for HPP in `bash $GITALK/utils/find_hpp.sh ../../src/cpp/elykseer-cpp.md`; do
  bash $GITALK/utils/make_cpp.sh ${HPP}
done

cd ..
cd test

$GITALK/utils/make_test.sh ../../tests/cpp/elykseer-cpp-ut.md

cd ..
