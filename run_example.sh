#!/bin/bash

basepwd=$(basename $PWD)
ninja $1 && (cd ../examples/$1 && ../../$basepwd/bin/$1*)
