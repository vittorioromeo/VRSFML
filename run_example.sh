#!/bin/bash

basepwd=$(basename $PWD)
cp compile_commands.json ..
ninja $1 && (cd ../examples/$1 && ../../$basepwd/bin/$1*)
