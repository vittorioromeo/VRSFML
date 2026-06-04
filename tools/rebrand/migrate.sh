#!/usr/bin/env bash
# Zancle migration -- sandbox-tested rename pipeline.
#
# Mutates $1 (default: sandbox/) in place. Always run reset.sh first if you
# want to start over. The intent is for this script to grow into the real
# migration: anything in here that handles an edge case is something we
# don't want to do by hand on the real repo.
#
# Operation order is load-bearing: more-specific patterns must run before
# less-specific ones so they don't get eaten. See comments at each step.
#
# For the real repo migration, replace `mv` with `git mv` in Phase 1 so
# blame history follows the file moves.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="${1:-sandbox}"
cd "$ROOT/$TARGET"

# --------------------------------------------------------------------------
# Phase 1 -- file moves.
#
# Promote Base out of the SFML/ tree FIRST (it becomes a sibling of the rest),
# then rename the remaining SFML/ tree to Zancle/. Same logic for src/.
# Use git mv on the real repo; here we use mv.
# --------------------------------------------------------------------------

move_tree() {
    # $1 = source dir relative to TARGET (e.g. include/SFML)
    # $2 = dest dir for Base content   (e.g. include/ZancleBase)
    # $3 = dest dir for everything else (e.g. include/Zancle)
    local src="$1" base_dst="$2" rest_dst="$3"
    [[ -d "$src" ]] || return 0

    if [[ -d "$src/Base" ]]; then
        mkdir -p "$base_dst"
        # Move every entry inside Base (including hidden subdirs like Trait/)
        # individually so we don't move-into-a-subdir if base_dst already exists.
        ( shopt -s dotglob nullglob; mv "$src/Base"/* "$base_dst"/ )
        rmdir "$src/Base"
    fi

    mkdir -p "$rest_dst"
    ( shopt -s dotglob nullglob; mv "$src"/* "$rest_dst"/ 2>/dev/null || true )
    rmdir "$src" 2>/dev/null || true
}

move_tree include/SFML  include/ZancleBase  include/Zancle
move_tree src/SFML      src/ZancleBase      src/Zancle

# --------------------------------------------------------------------------
# Phase 2 -- in-file substitutions.
#
# We collect every text file once into FILES, then run each substitution as
# a separate `perl -pi -e` invocation. Order matters: long, more-specific
# patterns first so they don't get cannibalised by the short ones.
#
# Word boundaries (\b) prevent damage to identifiers that just happen to
# contain `sf` or `SFML` as a substring (e.g. `sfmlLogo`, `Stuff`).
# --------------------------------------------------------------------------

mapfile -t FILES < <(find . -type f \( \
        -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
     -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
     -o -name '*.cmake' -o -name 'CMakeLists.txt'                        \
     -o -name '*.md'  -o -name '*.txt'                                   \
     -o -name '*.glsl' -o -name '*.frag' -o -name '*.vert' -o -name '*.geom' \
\) -print)

(( ${#FILES[@]} == 0 )) && { echo "No source files found"; exit 1; }

# CMake-only subset, defined here so steps below can use it.
mapfile -t CMAKE_FILES < <(find . -type f \( -name 'CMakeLists.txt' -o -name '*.cmake' \))

# Convenience wrapper so the substitution table reads as a list.
sub() { perl -i -pe "$1" "${FILES[@]}"; }

# -- 2a. Namespaces ---------------------------------------------------------
# Bare-namespace forms come first because they have no trailing `::` and
# would otherwise be mishandled (or missed) by the `\bsf::` rule below.
# Covers all three contexts where `sf` / `sf::base` appears without a
# trailing colon:
#   namespace sf { ... }            (declaration)
#   namespace sf::base { ... }      (nested declaration)
#   using namespace sf;             (using directive)
#   } // namespace sf::base::priv   (closing comment -- same regex still matches)
# Longer-form first so it isn't eaten by the shorter pattern.
sub 's/\bnamespace(\s+)sf::base\b/namespace${1}zb/g'
sub 's/\bnamespace(\s+)sf\b/namespace${1}za/g'

# Trailing-`::` forms: sf::base:: must precede sf:: so it isn't half-eaten.
sub 's/\bsf::base::/zb::/g'
sub 's/\bsf::/za::/g'

# -- 2a-bis. GLSL identifiers ----------------------------------------------
# VRSFML's public shader API uses `sf_u_*` (uniforms), `sf_a_*` (attributes),
# `sf_v_*` (varyings) and `sf_fragColor` (fragment output). These appear:
#   - As GLSL tokens inside C++ raw-string-literal shaders (DefaultShader.hpp)
#   - In Doxygen documentation about the shader contract
#   - In freestanding `.frag` / `.vert` / `.geom` test shaders
# Migrate them all so user shaders can be ported with a matching sed.
sub 's/\bsf_([uav])_/za_$1_/g'
sub 's/\bsf_fragColor\b/za_fragColor/g'

# -- 2a-ter. CamelCase `sfml<Upper>` identifiers ----------------------------
# C++ symbols that embed `sfml` as a prefix (e.g. internal ABI-check extern
# functions in Config.hpp: `sfmlInternalAbiCheckLifetimeTrackingEnabled`).
# Restricted to camelCase to avoid touching `"sfml_logo.png"` asset paths
# in string literals (snake_case stays -- left for human/asset review).
sub 's/\bsfml([A-Z])/zancle$1/g'

# -- 2b. Macros -------------------------------------------------------------
# Same ordering trap. Also catches header guards (#ifndef SFML_X) because the
# word boundary matches at column 0 too.
#
# Edge case: `-DSFML_FOO` (CMake `add_definitions` flag) is NOT caught by
# `\bSFML_` because there is no word boundary between `D` and `S` -- both are
# word characters. We add a dedicated pass for the `-D` prefix.
sub 's/-DSFML_BASE_/-DZB_/g'
sub 's/-DSFML_/-DZA_/g'
sub 's/\bSFML_BASE_/ZB_/g'
sub 's/\bSFML_/ZA_/g'

# -- 2c. Include paths ------------------------------------------------------
# Handles both `#include <SFML/Base/X>` and `"SFML/Base/X"` forms.
# Tolerates extra whitespace between `#`, `include`, and the path.
sub 's{#\s*include(\s*)<SFML/Base/}{#include$1<ZancleBase/}g'
sub 's{#\s*include(\s*)"SFML/Base/}{#include$1"ZancleBase/}g'
sub 's{#\s*include(\s*)<SFML/}{#include$1<Zancle/}g'
sub 's{#\s*include(\s*)"SFML/}{#include$1"Zancle/}g'

# -- 2d. CMake target / package names ---------------------------------------
# `SFML::ImGui`, `SFML::Graphics`, etc.  Also lowercase target names
# (`sfml-graphics`) used by some build systems internally.
sub 's/\bSFML::/Zancle::/g'
sub 's/\bsfml-/zancle-/g'

# Project-defined CMake helper macros: `sfml_add_example`, `sfml_set_option`,
# etc. These are lowercase snake_case identifiers, not C++ namespaces, so the
# `sf::` and `sfml-` patterns above don't catch them.
#
# Restricted to CMake files: a broader pass would mangle string literals like
# C++'s `"sfml_logo.png"` (asset filenames that have a separate decision --
# rename the file too, or leave it as historical content). Inside CMake we
# accept that asset paths can be touched; CMake review is unavoidable.
if (( ${#CMAKE_FILES[@]} > 0 )); then
    perl -i -pe 's/\bsfml_/zancle_/g' "${CMAKE_FILES[@]}"
fi

# -- 2e. Bare `SFML` token, CMake only --------------------------------------
# `project(SFML ...)`, `find_package(SFML ...)`, etc. Restrict to CMake files
# so we don't touch the brand name in C++ string literals or comments.
if (( ${#CMAKE_FILES[@]} > 0 )); then
    perl -i -pe 's/\bSFML\b/Zancle/g' "${CMAKE_FILES[@]}"
fi

echo "migrate.sh done. Run scripts/verify.sh (or tools/rebrand/verify.sh) to inspect."
