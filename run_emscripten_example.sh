#!/bin/bash

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$(cd "$(dirname "$0")" && pwd)/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

ninja "$1" || exit 1

for suffix in "-d.html" ".html"; do
    candidate="bin/$1$suffix"
    if [ -f "$candidate" ]; then
        found_target="$candidate"
        break
    fi
done

if [ -z "$found_target" ]; then
    echo "No matching Emscripten HTML found for $1 in ./bin/"
    exit 1
fi

# Kill any previous emrun server
if command -v taskkill &>/dev/null; then
    taskkill //F //IM python3.exe 2>/dev/null
    taskkill //F //IM python.exe 2>/dev/null
else
    killall python3 2>/dev/null
fi

emrun "$found_target" "${@:2}" &
