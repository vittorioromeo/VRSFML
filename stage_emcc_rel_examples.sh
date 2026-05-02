#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BIN_DIR="${SCRIPT_DIR}/build_vrdev_emcc_linux_rel/bin"
STAGING_DIR="${SCRIPT_DIR}/build_vrdev_emcc_linux_rel/staging"
COI_SW="${SCRIPT_DIR}/emscripten/coi-serviceworker.js"
EMBED_TEMPLATE="${SCRIPT_DIR}/emscripten/embed.html.in"

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

if [[ ! -f "${EMBED_TEMPLATE}" ]]; then
    echo "ERROR: ${EMBED_TEMPLATE} does not exist." >&2
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

    # Generate a minimal canvas-only embed wrapper for iframing into other
    # pages (e.g. the VRSFML landing page). It loads the same `<example>.js`
    # as the regular page, so no extra build / link step is required.
    sed "s|@SFML_EMBED_JS@|${example}.js|g" "${EMBED_TEMPLATE}" \
        > "${target_dir}/${example}_embed.html"

    copied=$((copied + 1))
    echo "Staged ${example} (+ embed)"
done

echo
echo "Done. Staged ${copied}/${#examples[@]} example(s) in ${STAGING_DIR}"

# Strip DWARF and other custom sections from the .wasm files. The build dir
# keeps the original (debug-info-heavy) wasm for in-browser debugging via the
# C/C++ DevTools extension; the staged copy we ship is lean. Typical reduction
# for our outputs: 16 MB -> ~4 MB (~70% of the .wasm is DWARF before stripping).
if STRIP_BIN="$(command -v wasm-strip)"; then
    echo
    JOBS="$(nproc 2>/dev/null || echo 4)"
    echo "Stripping DWARF / custom sections from .wasm files (${STRIP_BIN}, ${JOBS} parallel jobs) ..."

    mapfile -t to_strip < <(find "${STAGING_DIR}" -type f -name "*.wasm" -size +1024c -print)

    if [[ ${#to_strip[@]} -gt 0 ]]; then
        # Snapshot pre-strip sizes for the summary.
        declare -A pre_strip_size
        for f in "${to_strip[@]}"; do
            pre_strip_size["${f}"]=$(stat -c%s "${f}")
        done

        printf '%s\0' "${to_strip[@]}" \
            | xargs -0 -n1 -P"${JOBS}" "${STRIP_BIN}"

        raw_total=0
        stripped_total=0
        while IFS= read -r line; do
            raw=$(awk '{print $1}' <<<"${line}")
            stripped=$(awk '{print $2}' <<<"${line}")
            name=$(awk '{print $3}' <<<"${line}")
            raw_total=$((raw_total + raw))
            stripped_total=$((stripped_total + stripped))
            printf '  %-50s %10d -> %10d (%.0f%%)\n' \
                "${name}" "${raw}" "${stripped}" \
                "$(awk -v a="${stripped}" -v b="${raw}" 'BEGIN{printf "%.0f", (a/b)*100}')"
        done < <(
            for f in "${to_strip[@]}"; do
                printf '%s %s %s\n' \
                    "${pre_strip_size[$f]}" \
                    "$(stat -c%s "${f}")" \
                    "$(basename "${f}")"
            done | sort -n
        )

        if [[ ${raw_total} -gt 0 ]]; then
            echo
            printf 'Total: %d bytes pre-strip -> %d bytes stripped (%.0f%%)\n' \
                "${raw_total}" "${stripped_total}" \
                "$(awk -v a="${stripped_total}" -v b="${raw_total}" 'BEGIN{printf "%.0f", (a/b)*100}')"
        fi
    fi
else
    echo
    echo "WARN: \`wasm-strip\` not found in PATH -- skipping debug-info strip."
    echo "      Install it (e.g. \`pacman -S wabt\` / \`apt install wabt\`) for ~70% smaller .wasm files."
fi

# Pre-compress with brotli so static hosts that respect `Accept-Encoding: br`
# (Cloudflare Pages, itch.io, nginx with `brotli_static on`, ...) can serve
# the pre-built `.br` payloads instead of compressing on the fly. Typical
# wins for our outputs are ~70-80% on .wasm and ~75-85% on .js / .html.
# Hosts that don't honor pre-compressed files (e.g. GitHub Pages) just
# ignore the .br twins.
if BROTLI_BIN="$(command -v brotli)"; then
    echo
    JOBS="$(nproc 2>/dev/null || echo 4)"
    echo "Pre-compressing with brotli (${BROTLI_BIN}) using ${JOBS} parallel jobs ..."

    # Find every staged artifact worth compressing. Skip <1 KB files
    # (overhead dwarfs the saving) and anything already ending in .br
    # (the leading `rm -rf` already prevented stale .br files anyway).
    mapfile -t to_compress < <(find "${STAGING_DIR}" -type f \
        \( -name "*.html" -o -name "*.js" -o -name "*.wasm" \) \
        -size +1024c -print)

    if [[ ${#to_compress[@]} -gt 0 ]]; then
        # Compress in parallel: each file is independent, so wall-clock
        # scales with `nproc`. `--best` (level 11) stays in -- the extra
        # CPU is what we're trading away to get the smallest possible payload.
        # `--keep` leaves the raw file alongside; `--force` overwrites any
        # previously compressed twin (cheap insurance against partial reruns).
        printf '%s\0' "${to_compress[@]}" \
            | xargs -0 -n1 -P"${JOBS}" "${BROTLI_BIN}" --best --keep --force

        # Per-file size summary, sorted by raw size so the heaviest payload
        # (almost always the .wasm) lands at the bottom for easy spotting.
        raw_total=0
        br_total=0
        while IFS= read -r line; do
            raw=$(awk '{print $1}' <<<"${line}")
            br=$(awk '{print $2}' <<<"${line}")
            name=$(awk '{print $3}' <<<"${line}")
            raw_total=$((raw_total + raw))
            br_total=$((br_total + br))
            printf '  %-50s %8d -> %8d (%.0f%%)\n' \
                "${name}" "${raw}" "${br}" \
                "$(awk -v a="${br}" -v b="${raw}" 'BEGIN{printf "%.0f", (a/b)*100}')"
        done < <(
            for f in "${to_compress[@]}"; do
                printf '%s %s %s\n' \
                    "$(stat -c%s "${f}")" \
                    "$(stat -c%s "${f}.br")" \
                    "$(basename "${f}")"
            done | sort -n
        )

        if [[ ${raw_total} -gt 0 ]]; then
            echo
            printf 'Total: %d bytes raw -> %d bytes brotli (%.0f%%)\n' \
                "${raw_total}" "${br_total}" \
                "$(awk -v a="${br_total}" -v b="${raw_total}" 'BEGIN{printf "%.0f", (a/b)*100}')"
        fi
    fi
else
    echo
    echo "WARN: \`brotli\` not found in PATH -- skipping pre-compression."
    echo "      Install it (e.g. \`apt install brotli\` / \`pacman -S brotli\`) for ~70-80% smaller payloads."
fi

if [[ ${#missing[@]} -gt 0 ]]; then
    echo
    echo "Missing artifacts (these examples were skipped):"
    printf '  %s\n' "${missing[@]}"
    exit 2
fi
