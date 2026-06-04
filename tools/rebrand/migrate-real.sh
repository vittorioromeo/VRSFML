#!/usr/bin/env bash
# Zancle migration -- real-repo variant.
#
# Sister script to `migrate.sh` (which targets the sandbox layout). This one
# operates on the actual repository, uses `git mv` for file moves (so blame
# survives), and excludes build artefacts, vendored third-party code, and
# the migration tooling itself from the substitution sweep.
#
# Operation order is load-bearing: more-specific patterns must run before
# less-specific ones so they don't get eaten. See comments at each step.
#
# Run from the repository root, on a dedicated branch.

set -euo pipefail

# Sanity checks --------------------------------------------------------------
if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "ERROR: not inside a git working tree" >&2
    exit 1
fi

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "$REPO_ROOT"

BRANCH="$(git branch --show-current)"
if [[ "$BRANCH" == "master" || "$BRANCH" == "main" ]]; then
    echo "ERROR: refusing to run on $BRANCH. Switch to a dedicated rebrand branch first." >&2
    exit 1
fi

if [[ -n "$(git status --porcelain)" && -z "${ZANCLE_MIGRATE_ALLOW_DIRTY:-}" ]]; then
    echo "ERROR: working tree dirty. Commit or stash before running, or set" >&2
    echo "  ZANCLE_MIGRATE_ALLOW_DIRTY=1 to bypass (e.g. for an idempotent re-run)." >&2
    git status --short >&2
    exit 1
fi

echo "Running Zancle migration on branch: $BRANCH"
echo "Repository root: $REPO_ROOT"
echo

# --------------------------------------------------------------------------
# Phase 1 -- file moves with `git mv`.
# --------------------------------------------------------------------------

move_tree_git() {
    # $1 = source dir (e.g. include/SFML)
    # $2 = dest dir for Base content   (e.g. include/ZancleBase)
    # $3 = dest dir for everything else (e.g. include/Zancle)
    local src="$1" base_dst="$2" rest_dst="$3"
    [[ -d "$src" ]] || return 0

    # Promote Base/ out of the SFML tree first.
    if [[ -d "$src/Base" ]]; then
        mkdir -p "$base_dst"
        # `git mv` accepts <sources...> <destination dir>. Use a glob.
        # shellcheck disable=SC2086
        ( shopt -s dotglob nullglob; git mv $src/Base/* "$base_dst/" )
        rmdir "$src/Base"
    fi

    # Move whatever is left under src/ into the new namespaced location.
    mkdir -p "$rest_dst"
    # shellcheck disable=SC2086
    ( shopt -s dotglob nullglob
      # Iterate so we don't trip on git mv-ing zero arguments.
      shopt -s nullglob
      entries=( "$src"/* )
      if (( ${#entries[@]} > 0 )); then
          git mv "${entries[@]}" "$rest_dst/"
      fi
    )
    rmdir "$src" 2>/dev/null || true
}

echo "[Phase 1] File moves via git mv..."
move_tree_git include/SFML  include/ZancleBase  include/Zancle
move_tree_git src/SFML      src/ZancleBase      src/Zancle
echo "  done."
echo

# --------------------------------------------------------------------------
# Phase 2 -- in-file substitutions.
# Files in scope: source / docs / build files anywhere in the repo EXCEPT
# the danger zones (build artefacts, vendored third-party, our own scripts).
# --------------------------------------------------------------------------

echo "[Phase 2] Collecting files in scope..."
mapfile -t FILES < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs'    -o -path './.git' \
        -o -path './.cache'     -o -path './.vscode' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( \
            -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
         -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
         -o -name '*.mm'  -o -name '*.m'                                     \
         -o -name '*.cmake' -o -name '*.cmake.in' -o -name 'CMakeLists.txt'  \
         -o -name 'CMakePresets.json' -o -name 'CMakeUserPresets.json'       \
         -o -name '*.pc.in' -o -name '*.rc.in' -o -name 'doxyfile.in'        \
         -o -name '*.html.in'                                                \
         -o -name '*.md'  -o -name '*.txt'                                   \
         -o -name '*.sh'  -o -name 'Dockerfile'                              \
         -o -name '*.yml' -o -name '*.yaml'                                  \
         -o -name '*.kts' -o -name '*.gradle'                                \
         -o -name '*.glsl' -o -name '*.frag' -o -name '*.vert' -o -name '*.geom' \
    \) -print
)
echo "  ${#FILES[@]} files in scope."
echo

mapfile -t CMAKE_FILES < <(
    find . -type d \( \
           -path './build' -o -path './build_*' \
        -o -path './CMakeFiles' -o -path './Testing' \
        -o -path './extlibs'    -o -path './.git' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( -name 'CMakeLists.txt' -o -name '*.cmake' -o -name '*.cmake.in' \) -print
)

# perl -i -pe with explicit file list. xargs would chunk on too-many-args.
sub() {
    local expr="$1"
    # Use xargs to handle large argument lists safely.
    printf '%s\0' "${FILES[@]}" | xargs -0 perl -i -pe "$expr"
}

sub_cmake() {
    local expr="$1"
    (( ${#CMAKE_FILES[@]} == 0 )) && return 0
    printf '%s\0' "${CMAKE_FILES[@]}" | xargs -0 perl -i -pe "$expr"
}

# -- 2a. Namespaces ---------------------------------------------------------
echo "[Phase 2a] Namespaces (bare + trailing-::)..."
sub 's/\bnamespace(\s+)sf::base\b/namespace${1}zb/g'
sub 's/\bnamespace(\s+)sf\b/namespace${1}za/g'
sub 's/\bsf::base::/zb::/g'
sub 's/\bsf::/za::/g'
echo "  done."

# -- 2a-bis. GLSL identifiers ----------------------------------------------
echo "[Phase 2a-bis] GLSL identifiers..."
sub 's/\bsf_([uav])_/za_$1_/g'
sub 's/\bsf_fragColor\b/za_fragColor/g'
echo "  done."

# -- 2a-ter. CamelCase `sfml<Upper>` identifiers ----------------------------
echo "[Phase 2a-ter] CamelCase sfml<Upper> -> zancle<Upper>..."
sub 's/\bsfml([A-Z])/zancle$1/g'
echo "  done."

# -- 2b. Macros -------------------------------------------------------------
echo "[Phase 2b] Macros (incl. -D and bare forms)..."
sub 's/-DSFML_BASE_/-DZB_/g'
sub 's/-DSFML_/-DZA_/g'
sub 's/\bSFML_BASE_/ZB_/g'
sub 's/\bSFML_/ZA_/g'
echo "  done."

# -- 2c. Include paths ------------------------------------------------------
echo "[Phase 2c] Include paths..."
sub 's{#\s*include(\s*)<SFML/Base/}{#include$1<ZancleBase/}g'
sub 's{#\s*include(\s*)"SFML/Base/}{#include$1"ZancleBase/}g'
sub 's{#\s*include(\s*)<SFML/}{#include$1<Zancle/}g'
sub 's{#\s*include(\s*)"SFML/}{#include$1"Zancle/}g'
echo "  done."

# -- 2d. CMake target / package names ---------------------------------------
echo "[Phase 2d] CMake target names + helper macros..."
sub 's/\bSFML::/Zancle::/g'
sub 's/\bsfml-/zancle-/g'
sub_cmake 's/\bsfml_/zancle_/g'
echo "  done."

# -- 2e. Bare `SFML` token, CMake only --------------------------------------
echo "[Phase 2e] Bare SFML -> Zancle (CMake only)..."
sub_cmake 's/\bSFML\b/Zancle/g'
echo "  done."

# -- 2f. Long-tail mechanical cleanup ---------------------------------------
# (i) Objective-C `#import` mirrors `#include` for the same headers.
# (ii) Bare `<SFML/...>` paths in Doxygen comments / strings, scoped to code
#      files only so we don't rewrite historical content inside *.md docs.
# (iii) pkg-config `-lsfml-<module>` link flags must follow the library
#       rename or the linker breaks.
echo "[Phase 2f] Long-tail cleanup (#import, bare <SFML/>, pkg-config -l)..."

# Obj-C #import directives (mirror the include-path regexes).
sub 's{#\s*import(\s*)<SFML/Base/}{#import$1<ZancleBase/}g'
sub 's{#\s*import(\s*)"SFML/Base/}{#import$1"ZancleBase/}g'
sub 's{#\s*import(\s*)<SFML/}{#import$1<Zancle/}g'
sub 's{#\s*import(\s*)"SFML/}{#import$1"Zancle/}g'

# Bare <SFML/...> path references in Doxygen comments / error strings --
# scoped to CODE files so historical .md docs keep their original content.
mapfile -t CODE_FILES < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs'    -o -path './.git' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( \
            -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
         -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
         -o -name '*.mm'  -o -name '*.m' \
    \) -print
)
if (( ${#CODE_FILES[@]} > 0 )); then
    printf '%s\0' "${CODE_FILES[@]}" | xargs -0 perl -i -pe 's{<SFML/Base/}{<ZancleBase/}g'
    printf '%s\0' "${CODE_FILES[@]}" | xargs -0 perl -i -pe 's{<SFML/}{<Zancle/}g'
fi

# pkg-config -l<module> link flags (in *.pc.in templates).
mapfile -t PC_FILES < <(find tools/pkg-config -type f -name '*.pc.in' 2>/dev/null)
if (( ${#PC_FILES[@]} > 0 )); then
    printf '%s\0' "${PC_FILES[@]}" | xargs -0 perl -i -pe 's/-lsfml-/-lzancle-/g'
    printf '%s\0' "${PC_FILES[@]}" | xargs -0 perl -i -pe 's/Requires:\s*sfml-/Requires: zancle-/g'
fi

# User-agent string baked into Http.cpp (literal "libsfml-network/...").
if [[ -f src/Zancle/Network/Http.cpp ]]; then
    perl -i -pe 's/libsfml-/libzancle-/g' src/Zancle/Network/Http.cpp
fi
echo "  done."

echo "  done."

# -- 2g. Build-tested cleanups (discovered while making the rebrand compile) -
# These patterns were uncovered during the first real-repo build cycle and
# are mechanical enough to belong here. See README.md for the full story.
echo "[Phase 2g] Build-tested cleanups..."

# (i) `SFML<Upper>` glued-prefix identifiers: function names like
#     `mapSFMLScancodeToSDL`, `getSDLButtonFromSFMLButton`, and CMake EXPORT
#     names like `SFMLImGuiStaticTargets`. The bare `\bSFML\b` rule misses
#     these because there is no word boundary between `L` and the next upper
#     letter.
sub 's/\bSFML([A-Z])/Zancle$1/g'

# (ii) Sub-namespace shorthand: code used to write `base::Foo` inside
#      `namespace sf { ... }`, relying on the nested `sf::base` namespace
#      being visible. After promoting Base to the sibling top-level `zb::`,
#      bare `base::` no longer resolves and must become `zb::`.
#      Scoped to C++ source so we don't rewrite documentation prose that
#      mentions an unrelated "base::" by coincidence.
mapfile -t CPP_FILES < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs'    -o -path './.git' \
        -o -path './tools/rebrand' -o -path './webpage' \
    \) -prune -o -type f \( \
        -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \
     -o -name '*.inl' -o -name '*.mm' -o -name '*.m' \
    \) -print
)
if (( ${#CPP_FILES[@]} > 0 )); then
    printf '%s\0' "${CPP_FILES[@]}" | xargs -0 perl -i -pe 's/\bbase::/zb::/g'
fi

# (iii) Relative include paths peeking into private headers: tests use
#       `#include "../src/SFML/GLUtils/GlContext.hpp"` to access internals.
#       My `<SFML/...>` and `"SFML/..."` rules don't match the `../src/`
#       prefix.
sub 's{"\.\./src/SFML/}{"../src/Zancle/}g'

# (iv) BASE_INCROOT / BASE_SRCROOT in src/Zancle/System/CMakeLists.txt.
#      File moves promoted Base/ to top-level sibling (include/ZancleBase/,
#      src/ZancleBase/), but the CMake substitution `s/SFML/Zancle/g`
#      naively kept the nested path. Re-point to the sibling.
if [[ -f src/Zancle/System/CMakeLists.txt ]]; then
    perl -i -pe 's{(?<=include/)Zancle/Base\b}{ZancleBase}g; s{(?<=src/)Zancle/Base\b}{ZancleBase}g' \
        src/Zancle/System/CMakeLists.txt
fi

# (v) Test version vars: test/System/CMakeLists.txt set
#     `EXPECTED_ZA_VERSION_MAJOR=${ZA_VERSION_MAJOR}` after the rename, but
#     `ZA_VERSION_MAJOR` is not a CMake variable -- the CMake-magic one
#     follows the `project()` name (`Zancle_VERSION_MAJOR`) or use
#     `PROJECT_VERSION_MAJOR`.
if [[ -f test/System/CMakeLists.txt ]]; then
    perl -i -pe 's/\$\{ZA_VERSION_MAJOR\}/\${PROJECT_VERSION_MAJOR}/g;
                 s/\$\{ZA_VERSION_MINOR\}/\${PROJECT_VERSION_MINOR}/g;
                 s/\$\{ZA_VERSION_PATCH\}/\${PROJECT_VERSION_PATCH}/g' \
        test/System/CMakeLists.txt
fi

# (vi) Brand strings inside error-message prefixes: `[[SFML ERROR]]`,
#      `[[SFML ASSERTION FAILURE]]`, `[[SFML OPTIONAL FAILURE]]` are
#      user-visible at runtime and should follow the rebrand.
sub 's/\[\[SFML ERROR\]\]/[[ZANCLE ERROR]]/g'
sub 's/\[\[SFML ASSERTION FAILURE\]\]/[[ZANCLE ASSERTION FAILURE]]/g'
sub 's/\[\[SFML OPTIONAL FAILURE\]\]/[[ZANCLE OPTIONAL FAILURE]]/g'
echo "  done."

echo
echo "migrate-real.sh complete."
echo "Next: git status / git diff --stat, then build, then commit."
