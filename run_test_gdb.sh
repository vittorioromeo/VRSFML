#!/bin/bash

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$(cd "$(dirname "$0")" && pwd)/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

basepwd=$(basename $PWD)
ninja $1 && (cd ../test && gdb --args ../$basepwd/bin/$1* "${@:2}")
