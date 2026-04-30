#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BIN_DIR="${SCRIPT_DIR}/build_vrdev_emcc_linux_rel/bin"
STAGING_DIR="${SCRIPT_DIR}/build_vrdev_emcc_linux_rel/staging"
COI_SW="${SCRIPT_DIR}/emscripten/coi-serviceworker.js"

examples=(
    arkanoid
    batching
    block_puzzle
    coroutine
    coroutine_dialogue
    coroutine_tutorial
    event_handling
    imgui_minimal
    island
    joystick
    keyboard
    particles
    rockets
    shader
    showcase
    sound_effects
    stencil
    tennis
    tetris_survivors
)

if [[ ! -d "${BIN_DIR}" ]]; then
    echo "ERROR: ${BIN_DIR} does not exist -- run the Emscripten release build first." >&2
    exit 1
fi

if [[ ! -f "${COI_SW}" ]]; then
    echo "ERROR: ${COI_SW} does not exist." >&2
    exit 1
fi

# Wipe and recreate the staging directory so we never end up with stale artifacts
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}"

# coi-serviceworker.js is shared by every game and referenced from each .html
cp "${COI_SW}" "${STAGING_DIR}/"

missing=()
copied=0

for example in "${examples[@]}"; do
    target_dir="${STAGING_DIR}"
    mkdir -p "${target_dir}"

    # Each game needs its three artifacts (html / js / wasm). Skip .wasm.map -- it's
    # only useful for in-browser debugging and adds 1-3 MB per game.
    all_present=true
    for ext in html js wasm; do
        src="${BIN_DIR}/${example}.${ext}"
        if [[ ! -f "${src}" ]]; then
            missing+=("${example}.${ext}")
            all_present=false
        fi
    done

    if ! ${all_present}; then
        echo "WARN: skipping ${example} -- missing artifacts" >&2
        rmdir "${target_dir}" 2>/dev/null || true
        continue
    fi

    for ext in html js wasm; do
        cp "${BIN_DIR}/${example}.${ext}" "${target_dir}/${example}.${ext}"
    done

    # Each game's .html references coi-serviceworker.js relative to itself,
    # so drop a copy alongside (the per-directory copy is what the browser fetches).
    cp "${COI_SW}" "${target_dir}/"

    copied=$((copied + 1))
    echo "Staged ${example}"
done

echo
echo "Done. Staged ${copied}/${#examples[@]} example(s) in ${STAGING_DIR}"
if [[ ${#missing[@]} -gt 0 ]]; then
    echo "Missing artifacts (these examples were skipped):"
    printf '  %s\n' "${missing[@]}"
    exit 2
fi
