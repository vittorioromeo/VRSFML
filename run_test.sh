#!/bin/bash

basepwd=$(basename $PWD)
lowercase_arg=$(echo "$1" | tr '[:upper:]' '[:lower:]')
ninja "test-sfml-$lowercase_arg" && (cd "../test/$1" && ../../$basepwd/bin/test-sfml-$lowercase_arg* "${@:2}")
