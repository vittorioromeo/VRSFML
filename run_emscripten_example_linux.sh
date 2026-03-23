#!/bin/bash

ninja "$1" || exit 1

found_target=""

for suffix in ".html" "-d.html"; do
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

killall python3 2>/dev/null

/usr/lib/emscripten/emrun "$found_target" "${@:2}" &
