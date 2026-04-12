#!/bin/bash

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$(cd "$(dirname "$0")" && pwd)/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

lowercase_arg=$(echo "$1" | tr '[:upper:]' '[:lower:]')

ninja "test-sfml-$lowercase_arg" || exit 1

# Kill any previous emrun server
if command -v taskkill &>/dev/null; then
    taskkill //F //IM python3.exe 2>/dev/null
    taskkill //F //IM python.exe 2>/dev/null
else
    killall python3 2>/dev/null
fi

target="test-sfml-$lowercase_arg"

found=""
for suffix in "-d.html" ".html"; do
    candidate="bin/$target$suffix"
    if [ -f "$candidate" ]; then
        found="$candidate"
        break
    fi
done

if [ -n "$found" ]; then
    emrun "$found" -- "${@:2}"
else
    node ./bin/$target*.js "${@:2}"
fi
