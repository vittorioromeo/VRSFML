#!/usr/bin/env bash
# Modularize: split ZancleBase + Zancle/System into flat topical modules
# under include/Zancle/<Module>/ and src/Zancle/<Module>/.
#
# See MODULARIZATION_PLAN.md for the design and dependency hierarchy.
#
# Run from the repo root on a dedicated branch. Performs:
#   - git mv of every file from ZancleBase/ and Zancle/System/ into its
#     target module
#   - perl -i -pe rewrite of every include path repo-wide
#   - zb:: -> za::, ZB_ -> ZA_ across the codebase
#   - clean-up of the now-empty old roots
#
# Does NOT touch CMakeLists.txt (left for a follow-up).

set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT"

BRANCH="$(git branch --show-current)"
if [[ "$BRANCH" == "master" || "$BRANCH" == "main" ]]; then
    echo "ERROR: refusing to run on $BRANCH. Branch off first." >&2
    exit 1
fi

if [[ -n "$(git status --porcelain)" && -z "${ALLOW_DIRTY:-}" ]]; then
    echo "ERROR: working tree dirty. Commit or stash first," >&2
    echo "or set ALLOW_DIRTY=1 to bypass." >&2
    exit 1
fi

echo "=== Modularization starting on $BRANCH ==="

# --------------------------------------------------------------------------
# Module assignments
# --------------------------------------------------------------------------
# For each top-level ZancleBase file, declare its target module.
# Subdirectories (Trait/, Math/, Algorithm/, Fmt/, Scn/, Builtin/) are
# bulk-moved separately.

declare -A ZB_TO_MODULE=(
    # Diagnostic
    [Abort]=Diagnostic
    [Assert]=Diagnostic
    [AssertAndAssume]=Diagnostic

    # Math (top-level constants, clamp, min/max, etc.)
    [Clamp]=Math
    [ClampMacro]=Math
    [Constants]=Math
    [DoubleMax]=Math
    [FloatEpsilon]=Math
    [FloatMax]=Math
    [LongDoubleMax]=Math
    [MinMax]=Math
    [MinMaxMacros]=Math
    [Remainder]=Math
    [SinCosLookup]=Math

    # Vocabulary (single-value wrappers)
    [EnumClassBitwiseOps]=Vocabulary
    [FixedFunction]=Vocabulary
    [FunctionRef]=Vocabulary
    [InPlacePImpl]=Vocabulary
    [Optional]=Vocabulary
    [OverloadSet]=Vocabulary
    [PassKey]=Vocabulary
    [Radix]=Vocabulary
    [ScopeGuard]=Vocabulary
    [Span]=Vocabulary
    [UniquePtr]=Vocabulary
    [Variant]=Vocabulary

    # Container
    [AnkerlUnorderedDense]=Container
    [Array]=Container
    [BackInserter]=Container
    [Bitset]=Container
    [ChunkedVector]=Container
    [EnumArray]=Container
    [InPlaceVector]=Container
    [SmallVector]=Container
    [Vector]=Container

    # String + numeric I/O
    [FromChars]=String
    [FromCharsRadix]=String
    [FromCharsResult]=String
    [String]=String
    [StringStreamOp]=String
    [StringView]=String
    [StringViewSplits]=String
    [StringViewStreamOp]=String
    [ToChars]=String
    [ToCharsRadix]=String
    [ToString]=String

    # Chrono
    [StdChrono]=Chrono

    # Concurrency
    [ThreadPool]=Concurrency

    # Trait (single file outside Trait/ subdir)
    [MiniPFR]=Trait
    [RegularizeVoid]=Trait

    # Base (compiler intrinsics, primitive types, low-level macros)
    [DeclVal]=Base
    [Exchange]=Base
    [FloatTypes]=Base
    [FwdStdAlignedNewDelete]=Base
    [FwdStdHash]=Base
    [FwdStdLocale]=Base
    [FwdStdString]=Base
    [GetArraySize]=Base
    [IndexSequence]=Base
    [InitializerList]=Base
    [InterferenceSize]=Base
    [IntTypes]=Base
    [Launder]=Base
    [Macros]=Base
    [MakeIndexSequence]=Base
    [MaxAlignT]=Base
    [NonDeduced]=Base
    [PlacementNew]=Base
    [PtrDiffT]=Base
    [RequireDesignatedInitializers]=Base
    [SizeT]=Base
    [SourceLocation]=Base
    [Swap]=Base
    [TokenPaste]=Base
    [TrivialAbi]=Base
    [TypePackElement]=Base
    [TypePackIndex]=Base
    [UIntPtrT]=Base
)

# Zancle/System top-level header → module
declare -A SYS_TO_MODULE=(
    [Angle]=Geometry
    [AutoWrapAngle]=Geometry
    [Vec2]=Geometry
    [Vec3]=Geometry
    [Rect2]=Geometry
    [RectUtils]=Geometry
    [RectPacker]=Geometry

    [Time]=Chrono
    [Clock]=Chrono
    [SuspendAwareClock]=Chrono
    [TimeChronoUtil]=Chrono

    [Atomic]=Concurrency
    [AtomicMutex]=Concurrency
    [LockGuard]=Concurrency
    [Thread]=Concurrency

    [Utf]=String
    [Utf8String]=String
    [Utf8StringCodepoints]=String

    [InputStream]=IO
    [FileInputStream]=IO
    [MemoryInputStream]=IO
    [IO]=IO
    [Path]=IO
    [PathStreamOp]=IO

    [Err]=Err

    [LifetimeDependant]=Lifetime
    [LifetimeDependee]=Lifetime

    [GlobalAnchorPointMixin]=Mixin
    [LocalAnchorPointMixin]=Mixin

    [NativeActivity]=Window     # moves into the multimedia Window module
    [WindowsHeader]=Base        # cross-cutting glue, lives next to other Base helpers
)

# All seventeen modules
MODULES=(Base Trait Math Diagnostic Vocabulary Container Algorithm String
         Geometry Chrono Concurrency Fmt IO Scn Err Lifetime Mixin)

# --------------------------------------------------------------------------
# Phase 1: create destination directories
# --------------------------------------------------------------------------
echo "[Phase 1] Creating destination directories..."
for mod in "${MODULES[@]}"; do
    mkdir -p "include/Zancle/$mod/Priv"
    mkdir -p "src/Zancle/$mod"
done
# Window already exists (multimedia module), but make sure
mkdir -p "include/Zancle/Window"
echo "  done."
echo

# --------------------------------------------------------------------------
# Phase 2: file moves
# --------------------------------------------------------------------------

git_mv_if_exists() {
    local src="$1" dst="$2"
    if [[ -f "$src" ]]; then
        mkdir -p "$(dirname "$dst")"
        git mv "$src" "$dst"
    fi
}

# 2a. ZancleBase top-level headers
echo "[Phase 2a] Moving ZancleBase top-level headers..."
for stem in "${!ZB_TO_MODULE[@]}"; do
    mod="${ZB_TO_MODULE[$stem]}"
    git_mv_if_exists "include/ZancleBase/$stem.hpp" "include/Zancle/$mod/$stem.hpp"
    git_mv_if_exists "src/ZancleBase/$stem.cpp"    "src/Zancle/$mod/$stem.cpp"
    git_mv_if_exists "src/ZancleBase/$stem.hpp"    "src/Zancle/$mod/$stem.hpp"
done
echo "  done."

# 2b. ZancleBase/Trait/* → Zancle/Trait/*
echo "[Phase 2b] Moving ZancleBase/Trait/ → Zancle/Trait/..."
if [[ -d include/ZancleBase/Trait ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Trait/*; do
          git mv "$f" "include/Zancle/Trait/$(basename "$f")"
      done )
fi
echo "  done."

# 2c. ZancleBase/Math/* → Zancle/Math/* (preserve Priv/)
echo "[Phase 2c] Moving ZancleBase/Math/ → Zancle/Math/..."
if [[ -d include/ZancleBase/Math ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Math/*.hpp; do
          [[ -f "$f" ]] && git mv "$f" "include/Zancle/Math/$(basename "$f")"
      done
      if [[ -d include/ZancleBase/Math/Priv ]]; then
          for f in include/ZancleBase/Math/Priv/*; do
              git mv "$f" "include/Zancle/Math/Priv/$(basename "$f")"
          done
      fi )
fi
echo "  done."

# 2d. ZancleBase/Algorithm/* → Zancle/Algorithm/*
echo "[Phase 2d] Moving ZancleBase/Algorithm/ → Zancle/Algorithm/..."
if [[ -d include/ZancleBase/Algorithm ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Algorithm/*; do
          git mv "$f" "include/Zancle/Algorithm/$(basename "$f")"
      done )
fi
echo "  done."

# 2e. ZancleBase/Fmt/* → Zancle/Fmt/*
echo "[Phase 2e] Moving ZancleBase/Fmt/ → Zancle/Fmt/..."
if [[ -d include/ZancleBase/Fmt ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Fmt/*; do
          git mv "$f" "include/Zancle/Fmt/$(basename "$f")"
      done )
fi
if [[ -d src/ZancleBase/Fmt ]]; then
    ( shopt -s dotglob nullglob
      for f in src/ZancleBase/Fmt/*; do
          git mv "$f" "src/Zancle/Fmt/$(basename "$f")"
      done )
fi
echo "  done."

# 2f. ZancleBase/Scn/* → Zancle/Scn/*
echo "[Phase 2f] Moving ZancleBase/Scn/ → Zancle/Scn/..."
if [[ -d include/ZancleBase/Scn ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Scn/*; do
          git mv "$f" "include/Zancle/Scn/$(basename "$f")"
      done )
fi
if [[ -d src/ZancleBase/Scn ]]; then
    ( shopt -s dotglob nullglob
      for f in src/ZancleBase/Scn/*; do
          git mv "$f" "src/Zancle/Scn/$(basename "$f")"
      done )
fi
echo "  done."

# 2g. ZancleBase/Builtin/* → Zancle/Base/* (flat, no Builtin subfolder)
echo "[Phase 2g] Moving ZancleBase/Builtin/ → Zancle/Base/..."
if [[ -d include/ZancleBase/Builtin ]]; then
    ( shopt -s dotglob nullglob
      for f in include/ZancleBase/Builtin/*; do
          git mv "$f" "include/Zancle/Base/$(basename "$f")"
      done )
fi
echo "  done."

# 2h. ZancleBase/Priv/* (3 special-case files)
echo "[Phase 2h] Moving ZancleBase/Priv/* into their final locations..."
git_mv_if_exists "include/ZancleBase/Priv/ConstexprSinCos.hpp"   "include/Zancle/Math/Priv/ConstexprSinCos.hpp"
git_mv_if_exists "include/ZancleBase/Priv/SinCosLookupTable.inl" "include/Zancle/Math/Priv/SinCosLookupTable.inl"
git_mv_if_exists "include/ZancleBase/Priv/VectorUtils.hpp"       "include/Zancle/Container/Priv/VectorUtils.hpp"
echo "  done."

# 2i. Zancle/System top-level headers
echo "[Phase 2i] Moving Zancle/System headers..."
for stem in "${!SYS_TO_MODULE[@]}"; do
    mod="${SYS_TO_MODULE[$stem]}"
    git_mv_if_exists "include/Zancle/System/$stem.hpp" "include/Zancle/$mod/$stem.hpp"
    git_mv_if_exists "src/Zancle/System/$stem.cpp"    "src/Zancle/$mod/$stem.cpp"
    git_mv_if_exists "src/Zancle/System/$stem.hpp"    "src/Zancle/$mod/$stem.hpp"
done
echo "  done."

# 2j. Zancle/System/Priv/* (Vec2 helpers)
echo "[Phase 2j] Moving Zancle/System/Priv/ Vec2 helpers..."
git_mv_if_exists "include/Zancle/System/Priv/Vec2Base.hpp" "include/Zancle/Geometry/Priv/Vec2Base.hpp"
git_mv_if_exists "include/Zancle/System/Priv/Vec2Math.hpp" "include/Zancle/Geometry/Priv/Vec2Math.hpp"
echo "  done."

# 2k. Zancle/System/Fmt/FmtPath.hpp → Zancle/Err/FmtPath.hpp
echo "[Phase 2k] Moving Zancle/System/Fmt/FmtPath.hpp..."
git_mv_if_exists "include/Zancle/System/Fmt/FmtPath.hpp" "include/Zancle/Err/FmtPath.hpp"
echo "  done."

# 2l. Drop the System/Export.hpp; each module will eventually get its own
#     Export.hpp. For now we keep no Export.hpp anywhere -- the API macros
#     have already been migrated by previous rebrand passes.
echo "[Phase 2l] Removing Zancle/System/Export.hpp (per-module Export.hpp follow-up)..."
if [[ -f include/Zancle/System/Export.hpp ]]; then
    git rm include/Zancle/System/Export.hpp
fi
echo "  done."

# 2m. Remaining src/ files that I didn't catch by header name lookup
echo "[Phase 2m] Sweep stragglers in src/ZancleBase and src/Zancle/System..."
# Anything still left under src/ZancleBase or src/Zancle/System whose stem
# matches one of our module assignments
straggle() {
    local src_dir="$1"
    local strip_prefix="$2"
    local -n assoc="$3"
    [[ -d "$src_dir" ]] || return 0
    find "$src_dir" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.inl' \) | while read -r f; do
        rel="${f#$strip_prefix/}"
        stem="${rel%.*}"
        # The stem may be a path with subdirs (e.g. Unity/Foo)
        base="${stem##*/}"
        # Look up base name in the assoc array
        mod="${assoc[$base]:-}"
        if [[ -n "$mod" ]]; then
            # preserve any subpath relative to the source dir under the new module
            mkdir -p "$(dirname "src/Zancle/$mod/$rel")"
            git mv "$f" "src/Zancle/$mod/$rel"
        fi
    done
}
straggle src/ZancleBase    src/ZancleBase    ZB_TO_MODULE
straggle src/Zancle/System src/Zancle/System SYS_TO_MODULE
echo "  done."

echo

# --------------------------------------------------------------------------
# Phase 3: include-path rewrites
# --------------------------------------------------------------------------
echo "[Phase 3] Rewriting include paths..."

mapfile -t TEXT_FILES < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs' -o -path './.git' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( \
            -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
         -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
         -o -name '*.mm'  -o -name '*.m'                                     \
         -o -name '*.cmake' -o -name '*.cmake.in' -o -name 'CMakeLists.txt'  \
         -o -name '*.md'  -o -name '*.txt'                                   \
    \) -print
)
echo "  ${#TEXT_FILES[@]} files in scope"

rewrite() {
    # rewrite "<old_path>" "<new_path>" -- escapes regex chars in old
    local old="$1" new="$2"
    printf '%s\0' "${TEXT_FILES[@]}" | xargs -0 perl -i -pe "s{\\Q${old}\\E}{${new}}g"
}

# 3a. Specific Priv mappings (must come BEFORE the catch-all subdir rules)
rewrite "ZancleBase/Priv/ConstexprSinCos.hpp"   "Zancle/Math/Priv/ConstexprSinCos.hpp"
rewrite "ZancleBase/Priv/SinCosLookupTable.inl" "Zancle/Math/Priv/SinCosLookupTable.inl"
rewrite "ZancleBase/Priv/VectorUtils.hpp"       "Zancle/Container/Priv/VectorUtils.hpp"

# 3b. ZancleBase/Builtin/* → Zancle/Base/*
rewrite "ZancleBase/Builtin/" "Zancle/Base/"

# 3c. ZancleBase subdirectories that map 1:1
rewrite "ZancleBase/Trait/"     "Zancle/Trait/"
rewrite "ZancleBase/Math/"      "Zancle/Math/"
rewrite "ZancleBase/Algorithm/" "Zancle/Algorithm/"
rewrite "ZancleBase/Fmt/"       "Zancle/Fmt/"
rewrite "ZancleBase/Scn/"       "Zancle/Scn/"

# 3d. Zancle/System/Priv/Vec2* → Zancle/Geometry/Priv/Vec2*
rewrite "Zancle/System/Priv/Vec2Base.hpp" "Zancle/Geometry/Priv/Vec2Base.hpp"
rewrite "Zancle/System/Priv/Vec2Math.hpp" "Zancle/Geometry/Priv/Vec2Math.hpp"

# 3e. Zancle/System/Fmt/FmtPath.hpp → Zancle/Err/FmtPath.hpp
rewrite "Zancle/System/Fmt/FmtPath.hpp" "Zancle/Err/FmtPath.hpp"

# 3f. ZancleBase top-level files: each file maps to its module's directory.
for stem in "${!ZB_TO_MODULE[@]}"; do
    mod="${ZB_TO_MODULE[$stem]}"
    rewrite "ZancleBase/$stem.hpp" "Zancle/$mod/$stem.hpp"
done

# 3g. Zancle/System top-level files
for stem in "${!SYS_TO_MODULE[@]}"; do
    mod="${SYS_TO_MODULE[$stem]}"
    rewrite "Zancle/System/$stem.hpp" "Zancle/$mod/$stem.hpp"
done

# 3h. Drop any include of the gone Zancle/System/Export.hpp
printf '%s\0' "${TEXT_FILES[@]}" | xargs -0 perl -i -pe \
    's{^#\s*include\s+"Zancle/System/Export\.hpp"\s*\n}{}g'

echo "  done."
echo

# --------------------------------------------------------------------------
# Phase 4: namespace rename  zb:: -> za::
# --------------------------------------------------------------------------
echo "[Phase 4] zb:: -> za::, namespace zb -> namespace za..."
mapfile -t CODE_FILES < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs' -o -path './.git' \
        -o -path './tools/rebrand' -o -path './webpage' \
    \) -prune -o -type f \( \
        -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \
     -o -name '*.inl' -o -name '*.mm' -o -name '*.m'                  \
    \) -print
)
if (( ${#CODE_FILES[@]} > 0 )); then
    printf '%s\0' "${CODE_FILES[@]}" | xargs -0 perl -i -pe '
        s/\bnamespace(\s+)zb\b/namespace${1}za/g;
        s/\bzb::/za::/g;
    '
fi
echo "  done."

# --------------------------------------------------------------------------
# Phase 5: macro rename  ZB_ -> ZA_
# --------------------------------------------------------------------------
echo "[Phase 5] ZB_* -> ZA_*..."
mapfile -t TEXT_FILES_ALL < <(
    find . -type d \( \
           -name 'build' -o -name 'build_*' \
        -o -name 'CMakeFiles' -o -name 'Testing' \
        -o -path './extlibs' -o -path './.git' \
        -o -path './tools/rebrand' \
    \) -prune -o -type f \( \
            -name '*.hpp' -o -name '*.cpp' -o -name '*.h'   -o -name '*.c'   \
         -o -name '*.inl' -o -name '*.cc'  -o -name '*.hh'                   \
         -o -name '*.mm'  -o -name '*.m'                                     \
         -o -name '*.cmake' -o -name '*.cmake.in' -o -name 'CMakeLists.txt'  \
         -o -name '*.md'  -o -name '*.txt'                                   \
    \) -print
)
if (( ${#TEXT_FILES_ALL[@]} > 0 )); then
    printf '%s\0' "${TEXT_FILES_ALL[@]}" | xargs -0 perl -i -pe '
        s/-DZB_/-DZA_/g;
        s/\bZB_/ZA_/g;
    '
fi
echo "  done."

# --------------------------------------------------------------------------
# Phase 6: retire old roots if empty
# --------------------------------------------------------------------------
echo "[Phase 6] Removing empty old roots..."
# Try to remove old directories that should now be empty
find include/ZancleBase -depth -type d -empty -delete 2>/dev/null || true
find src/ZancleBase     -depth -type d -empty -delete 2>/dev/null || true
find include/Zancle/System -depth -type d -empty -delete 2>/dev/null || true
find src/Zancle/System -depth -type d -empty -delete 2>/dev/null || true

for d in include/ZancleBase src/ZancleBase include/Zancle/System src/Zancle/System; do
    if [[ -e "$d" ]]; then
        echo "  WARNING: $d still exists (non-empty?). Inspect manually."
    fi
done
echo "  done."
echo

echo "=== Modularization complete. ==="
echo "Next steps:"
echo "  1. Update src/Zancle/CMakeLists.txt + the per-module CMake configs"
echo "  2. Rebuild and fix compile errors"
echo "  3. Update .clang-format IncludeCategories"
