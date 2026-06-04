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

if [[ -n "$(git status --porcelain)" ]]; then
    echo "ERROR: working tree dirty. Commit or stash before running." >&2
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
           -path './build' -o -path './build_*' \
        -o -path './CMakeFiles' -o -path './Testing' \
        -o -path './extlibs'    -o -path './.git' \
        -o -path './.cache'     -o -path './.vscode' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( \
            -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
         -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
         -o -name '*.cmake' -o -name 'CMakeLists.txt'                        \
         -o -name 'CMakePresets.json' -o -name 'CMakeUserPresets.json'       \
         -o -name '*.md'  -o -name '*.txt'                                   \
         -o -name '*.sh'  -o -name 'Dockerfile'                              \
         -o -name '*.yml' -o -name '*.yaml'                                  \
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
    \) -prune -o -type f \( -name 'CMakeLists.txt' -o -name '*.cmake' \) -print
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

echo
echo "migrate-real.sh complete."
echo "Next: git status / git diff --stat, then build, then commit."
