#!/bin/bash

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$(cd "$(dirname "$0")" && pwd)/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

basepwd=$(basename "$PWD")
cp compile_commands.json ..
ninja "$1" && (
  cd "../examples/$1" || exit
  for suffix in ".exe" "-d.exe" "" "-d"; do
    executable="../../$basepwd/bin/$1$suffix"
    if [ -x "$executable" ]; then
       gdb -ex run "$executable"
      exit
    fi
  done
  echo "No matching executable found for $1"
)
