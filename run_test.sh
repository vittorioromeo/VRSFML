#!/bin/bash

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$(cd "$(dirname "$0")" && pwd)/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

basepwd=$(basename $PWD)
lowercase_arg=$(echo "$1" | tr '[:upper:]' '[:lower:]')
ninja "test-sfml-$lowercase_arg" && (cd "../test/$1" && ../../$basepwd/bin/test-sfml-$lowercase_arg* "${@:2}")
