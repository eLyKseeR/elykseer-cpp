#!/usr/bin/env bash

GITALK=`pwd`/../ext/gitalk

mkdir -p src/lxr

cd src
cd lxr
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/elykseer-cpp.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
done

cd ..
for HPP in `bash $GITALK/utils/find_hpp.sh ../../src/cpp/elykseer-cpp.md`; do
  bash $GITALK/utils/make_cpp.sh ${HPP}
done

cd cli
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/lxrbackup.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
  bash $GITALK/utils/make_cpp.sh ${HPP}
done
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/lxrrestore.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
  bash $GITALK/utils/make_cpp.sh ${HPP}
done
for HPP in `bash $GITALK/utils/find_hpp.sh ../../../src/cpp/lxrverify.md`; do
  bash $GITALK/utils/make_hpp.sh ${HPP}
  bash $GITALK/utils/make_cpp.sh ${HPP}
done
cd ..

cd ..
#cd test

#$GITALK/utils/make_test.sh ../../tests/cpp/elykseer-cpp-ut.md

#cd ..
