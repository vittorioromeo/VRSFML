#!/bin/bash
#
# Run all test suites for the active build directory.
#
# - On native builds, just delegates to the `runtests` ninja target which
#   ctest-runs everything.
# - On Emscripten builds (build dir name contains "emcc"), prompts per-suite
#   and invokes the merged `run_test.sh` to launch each in turn -- there is
#   no equivalent of `runtests` for browser-based tests.
#
# Env:
#   DEBUGGER   forwarded to per-suite invocations on native builds via
#              run_test.sh. Ignored on Emscripten.

set -u

script_dir="$(cd "$(dirname "$0")" && pwd)"
basepwd=$(basename "$PWD")

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$script_dir/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

if [[ "$basepwd" == *emcc* ]]; then
    suites=(
        Audio
        Base
        GLUtils
        Graphics
        Network
        System
        Window
    )

    for s in "${suites[@]}"; do
        read -rn1 -p "Run test suite '${s}'? [Y/n/q] " answer </dev/tty
        echo
        case "$answer" in
            q|Q) echo "Quitting."; exit 0 ;;
            n|N) echo "[skip] ${s}"; continue ;;
        esac

        echo "[run]  ${s}"
        "$script_dir/run_test.sh" "$s" "${@:1}" || echo "[warn] ${s} exited with code $?"
    done
    echo "Done."
else
    ninja runtests "$@"
fi
