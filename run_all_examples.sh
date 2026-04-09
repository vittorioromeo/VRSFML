#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export LSAN_OPTIONS="${LSAN_OPTIONS:+$LSAN_OPTIONS:}suppressions=${SCRIPT_DIR}/lsan_suppressions.txt"
export ASAN_SYMBOLIZER_PATH="/opt/emscripten-llvm/bin/llvm-symbolizer"

examples=(
    arkanoid
    batching
    batching_benchmark
    block_puzzle
    bubble_idle
    event_handling
    # ftp
    # http
    imgui_minimal
    imgui_multiple_windows
    island
    joystick
    keyboard
    particles
    raw_input
    rockets
    shader
    showcase
    # sockets
    # sound
    # sound_capture
    sound_effects
    # sound_multi_device
    stencil
    tennis
    tetris_survivors
    # text_benchmark
    # voip
    # vulkan
)

for example in "${examples[@]}"; do
    read -rn1 -p "Run '${example}'? [Y/n/q] " answer </dev/tty
    echo
    case "$answer" in
        q|Q) echo "Quitting."; exit 0 ;;
        n|N) echo "[skip] ${example}"; continue ;;
    esac

    echo "[run]  ${example}"
    "${SCRIPT_DIR}/run_example.sh" "$example" || echo "[warn] ${example} exited with code $?"
done

echo "Done."
