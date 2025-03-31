#!/bin/bash

basepwd=$(basename $PWD)
ninja $1 && (cd ../test && gdb --args ../$basepwd/bin/$1* "${@:2}")
