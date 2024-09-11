#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

NAME1=$1
NAME2=${NAME1//-/_}
NAME3=${NAME1//_/-}

(cd $SCRIPT_DIR/build_vrdev_clang && ninja) && (cd $SCRIPT_DIR/examples/$NAME1 && $SCRIPT_DIR/build_vrdev_clang/bin/$NAME1*.exe || $SCRIPT_DIR/build_vrdev_clang/bin/$NAME2*.exe || $SCRIPT_DIR/build_vrdev_clang/bin/$NAME3*.exe)
