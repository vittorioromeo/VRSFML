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

# When Chrome is available, launch via emrun's `--browser=chrome` so we can
# pass `--auto-open-devtools-for-tabs`. Set `EMRUN_NO_DEVTOOLS=1`` to skip this.
chrome_browser_args="--auto-open-devtools-for-tabs"

if [ -n "$found" ]; then
    if [ -z "$EMRUN_NO_DEVTOOLS" ] && emrun --list-browsers 2>&1 | grep -qi '^\s*-\s*chrome\b'; then
        emrun --browser=chrome --browser-args="$chrome_browser_args" "$found" -- "${@:2}"
    else
        emrun "$found" -- "${@:2}"
    fi
else
    node --stack-trace-limit=200 ./bin/$target*.js "${@:2}"
fi
