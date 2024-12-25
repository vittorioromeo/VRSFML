#!/bin/bash

basepwd=$(basename $PWD)
ninja $1 && (cd ../examples/$1 && gdb -ex run ../../$basepwd/bin/$1*)
