#!/bin/bash

GITALK=`pwd`/../ext/gitalk

cd src

cd lxr
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/elykseer-base.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
done

cd ..
for HPP in `bash $GITALK/utils/find_hpp.sh ../../src/cpp/elykseer-base.md`; do
  bash $GITALK/utils/make_cpp.sh ${HPP}
done

cd ..
cd tests

$GITALK/utils/make_test.sh ../../tests/cpp/elykseer-base-ut.md

cd ..
