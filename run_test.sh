#!/bin/bash

basepwd=$(basename $PWD)
ninja $1 && (cd ../test && ../$basepwd/bin/$1* "${@:2}")
