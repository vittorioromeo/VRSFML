#!/bin/bash
#
# Build and run a single example. Detects the active build directory by name:
# if it contains "emcc", uses emrun against the generated HTML; otherwise
# runs the native binary directly.
#
# Usage:
#   ./run_example.sh <example> [args...]
#
# Env:
#   DEBUGGER   command to prefix the binary with on native builds.
#              Examples:
#                DEBUGGER='gdb --args' ./run_example.sh foo
#                DEBUGGER='gdb -ex run --args' ./run_example.sh foo
#                DEBUGGER=lldb ./run_example.sh foo            # type `run` inside lldb
#                DEBUGGER='lldb --' ./run_example.sh foo
#                DEBUGGER='rr record' ./run_example.sh foo
#              Ignored on Emscripten builds.

set -u

target="${1:-}"
if [ -z "$target" ]; then
    sed -n '3,18p' "$0" >&2
    exit 1
fi

script_dir="$(cd "$(dirname "$0")" && pwd)"
basepwd=$(basename "$PWD")

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$script_dir/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

if [[ "$basepwd" == *emcc* ]]; then
    # --- Emscripten path ---
    ninja "$target" || exit 1

    # Stage coi-serviceworker.js next to the html so the page can register
    # the SharedArrayBuffer-enabling COOP/COEP headers via the service worker
    # (see emscripten/shell.html). emrun's static server doesn't set them.
    cp -f "$script_dir/emscripten/coi-serviceworker.js" bin/

    found=""
    for suffix in "-d.html" ".html"; do
        candidate="bin/$target$suffix"
        if [ -f "$candidate" ]; then
            found="$candidate"
            break
        fi
    done

    if [ -z "$found" ]; then
        echo "No matching Emscripten HTML found for $target in ./bin/" >&2
        exit 1
    fi

    [ -n "${DEBUGGER:-}" ] && echo "Note: DEBUGGER ignored for Emscripten builds." >&2

    # Kill any previous emrun server
    if command -v taskkill &>/dev/null; then
        taskkill //F //IM python3.exe 2>/dev/null
        taskkill //F //IM python.exe 2>/dev/null
    else
        killall python3 2>/dev/null
    fi

    emrun "$found" "${@:2}" &
else
    # --- Native path ---
    [ -f compile_commands.json ] && cp compile_commands.json ..

    ninja "$target" || exit 1
    cd "../examples/$target" || exit 1

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
