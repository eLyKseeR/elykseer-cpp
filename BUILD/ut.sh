#!/bin/sh

D=$TMPDIR/restored
test -d $D || mkdir $D
D=$TMPDIR/meta
test -d $D || mkdir $D

# --log_level=all --run_test=utRestoreCtrl

#./test/utelykseer-cpp --show_progress  $*

./test/utelykseer-cpp --no_color_output  $*
