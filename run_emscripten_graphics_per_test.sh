#!/bin/bash
#
# Run each Graphics test case in its own browser tab so that cumulative
# SDL/Emscripten state never builds up across cases. Mirrors run_test.sh
# but loops over individual `-tc=` filters.
#
# Usage:
#   ./run_emscripten_graphics_per_test.sh                      # run every TEST_CASE
#   ./run_emscripten_graphics_per_test.sh '*Sprite*'           # only matching cases
#
# Env:
#   EMRUN_NO_KILL=1   leave the browser open after each test (debugging)

set -u

script_dir="$(cd "$(dirname "$0")" && pwd)"

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=$script_dir/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

target="test-sfml-graphics"

ninja "$target" || exit 1

found=""
for suffix in "-d.html" ".html"; do
    candidate="bin/$target$suffix"
    if [ -f "$candidate" ]; then
        found="$candidate"
        break
    fi
done

if [ -z "$found" ]; then
    echo "No .html build for $target found in bin/ -- Emscripten target not built?" >&2
    exit 1
fi

filter_pattern="${1:-*}"

# Collect TEST_CASE names from test/Graphics/*.cpp. We grep the first string
# literal after TEST_CASE( and ignore decorators like `* tst::skip(...)`.
mapfile -t test_names < <(
    grep -hE '^TEST_CASE\s*\(\s*"' "$script_dir"/test/Graphics/*.cpp \
        | sed -E 's/^TEST_CASE\s*\(\s*"([^"]+)".*/\1/' \
        | sort -u
)

if [ "${#test_names[@]}" -eq 0 ]; then
    echo "No TEST_CASE entries found in test/Graphics/*.cpp" >&2
    exit 1
fi

# Per-run isolated Chrome profile -- prevents the loop from clobbering the
# user's normal browser session and lets --kill_exit terminate cleanly.
profile_root="$(mktemp -d -t vrsfml-emrun-XXXXXX)"
trap 'rm -rf "$profile_root"' EXIT

emrun_kill_flag="--kill_exit"
[ -n "${EMRUN_NO_KILL:-}" ] && emrun_kill_flag=""

passed=()
failed=()
skipped=()

idx=0
total="${#test_names[@]}"
for name in "${test_names[@]}"; do
    idx=$((idx + 1))

    # Apply user-supplied filter (shell glob-style).
    case "$name" in
        $filter_pattern) ;;
        *) skipped+=("$name"); continue ;;
    esac

    echo
    echo "=========================================================="
    echo "[$idx/$total] $name"
    echo "=========================================================="

    profile_dir="$profile_root/$idx"
    mkdir -p "$profile_dir"

    # Pass the test-case filter as a single `-tc=<name>` token (per shell.html
    # `Module.arguments` plumbing). Quote the name so spaces survive.
    if emrun \
        --browser=chrome \
        --browser-args="--user-data-dir=$profile_dir --no-first-run --no-default-browser-check" \
        $emrun_kill_flag \
        "$found" -- "-tc=$name"
    then
        passed+=("$name")
    else
        failed+=("$name")
    fi
done

echo
echo "=========================================================="
echo "Summary"
echo "=========================================================="
echo "Passed:  ${#passed[@]}"
echo "Failed:  ${#failed[@]}"
echo "Skipped: ${#skipped[@]} (filter: $filter_pattern)"

if [ "${#failed[@]}" -gt 0 ]; then
    echo
    echo "Failed cases:"
    for n in "${failed[@]}"; do echo "  - $n"; done
    exit 1
fi
