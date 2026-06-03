#!/bin/bash
#
# Build and run a single test suite. Detects the active build directory by
# name: if it contains "emcc", uses emrun against the generated HTML (or
# falls back to node); otherwise runs the native test binary directly.
#
# Usage:
#   ./run_test.sh <Suite> [test-runner-args...]
#   e.g.  ./run_test.sh Graphics -tc='*Sprite*'
#
# Env:
#   DEBUGGER   command to prefix the binary with on native builds.
#              Examples:
#                DEBUGGER='gdb --args' ./run_test.sh Graphics
#                DEBUGGER=lldb ./run_test.sh Graphics
#              Ignored on Emscripten builds.

set -u

suite="${1:-}"
if [ -z "$suite" ]; then
    sed -n '3,16p' "$0" >&2
    exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
basepwd=$(basename "$PWD")
lowercase_arg=$(echo "$suite" | tr '[:upper:]' '[:lower:]')
target="test-sfml-$lowercase_arg"

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$script_dir/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

if [[ "$basepwd" == *emcc* ]]; then
    # --- Emscripten path ---
    ninja "$target" || exit 1

    # Stage coi-serviceworker.js next to the html so the page can register
    # the SharedArrayBuffer-enabling COOP/COEP headers via the service worker
    # (see emscripten/shell.html). emrun's static server doesn't set them.
    cp -f "$script_dir/emscripten/coi-serviceworker.js" bin/

    [ -n "${DEBUGGER:-}" ] && echo "Note: DEBUGGER ignored for Emscripten builds." >&2

    # Kill any previous emrun server
    if command -v taskkill &>/dev/null; then
        taskkill //F //IM python3.exe 2>/dev/null
        taskkill //F //IM python.exe 2>/dev/null
    else
        killall python3 2>/dev/null
    fi

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
        node --stack-trace-limit=200 ./bin/${target}*.js "${@:2}"
    fi
else
    # --- Native path ---
    ninja "$target" || exit 1
    cd "../test/$suite" || exit 1

    for suffix in ".exe" "-d.exe" "" "-d"; do
        executable="../../$basepwd/bin/$target$suffix"
        if [ -x "$executable" ]; then
            ${DEBUGGER:-} "$executable" "${@:2}"
            exit
        fi
    done

    echo "No matching executable found for $target" >&2
    exit 1
fi
