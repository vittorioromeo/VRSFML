#!/bin/bash
#
# Build & test six cmake presets in parallel tmux panes.
#
#   ./run_all_test_variants.sh
#
# Each pane runs: cmake configure (--preset) -> cmake --build -> ctest.
# When all three succeed, the pane prints `RESULT: <preset> PASS` and waits
# for Enter so you can scroll the output. A failure at any step prints
# `RESULT: <preset> FAILED (configure|build|tests)` and waits the same way.

set -u

PRESETS=(
    vrdev_clang
    vrdev_clang_rel
    vrdev_gcc
    vrdev_gcc_rel
    vrdev_clang_egl
    vrdev_clang_egl_rel
)

# Cap each build's parallelism so the six panes don't oversubscribe the box.
# nproc / number-of-panes, floored at 2.
NPROC="$(nproc)"
PER_PANE_JOBS=$(( NPROC / ${#PRESETS[@]} ))
[ "$PER_PANE_JOBS" -lt 2 ] && PER_PANE_JOBS=2

ROOT="$(cd "$(dirname "$0")" && pwd)"
SESSION="vrsfml_test_all"

# --pane <preset>: actual work for one pane. Recursive self-invocation.
if [ "${1:-}" = "--pane" ]; then
    preset="$2"
    builddir="$ROOT/build_$preset"
    jobs="$3"

    set -o pipefail

    fail() {
        echo
        echo "RESULT: $preset FAILED ($1)"
        read -r -p "[press Enter to close pane] "
        exit 1
    }

    echo "=== [$preset] configure  (jobs=$jobs) ==="
    cmake --preset "$preset" || fail configure

    echo
    echo "=== [$preset] build ==="
    cmake --build "$builddir" --parallel "$jobs" || fail build

    echo
    echo "=== [$preset] test ==="
    ctest --test-dir "$builddir" \
          --output-on-failure \
          --no-tests=error \
          --parallel "$jobs" || fail tests

    echo
    echo "RESULT: $preset PASS"
    read -r -p "[press Enter to close pane] "
    exit 0
fi

# --- driver ---

if ! command -v tmux >/dev/null 2>&1; then
    echo "tmux not installed. On Arch: sudo pacman -S tmux" >&2
    exit 1
fi

self="$(realpath "$0")"

# Drop any stale session of the same name.
tmux kill-session -t "$SESSION" 2>/dev/null

# Create session with the first preset's pane.
tmux new-session -d -s "$SESSION" -n "tests" -x 240 -y 60 \
    "$self --pane ${PRESETS[0]} $PER_PANE_JOBS"

# Split off the rest, retiling after each split so panes stay roughly equal.
for ((i = 1; i < ${#PRESETS[@]}; i++)); do
    tmux split-window -t "$SESSION:tests" \
        "$self --pane ${PRESETS[$i]} $PER_PANE_JOBS"
    tmux select-layout -t "$SESSION:tests" tiled
done

# Pane border = preset name, so the layout self-documents.
tmux set -t "$SESSION" pane-border-status top
for ((i = 0; i < ${#PRESETS[@]}; i++)); do
    tmux select-pane -t "$SESSION:tests.$i" -T "${PRESETS[$i]}"
done

tmux select-pane -t "$SESSION:tests.0"
tmux attach-session -t "$SESSION"
