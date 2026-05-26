#!/usr/bin/env bash
# Post-migration verification: surface anything in sandbox/ that looks like
# a missed rename. Anything that prints here needs human (or agent) review
# before the real migration ships.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT/sandbox"

heading() { printf '\n=== %s ===\n' "$1"; }

heading "Old top-level paths that should not exist"
for p in include/SFML src/SFML; do
    [[ -e "$p" ]] && echo "  STILL EXISTS: $p"
done

heading "Files containing 'sf::' (should be 0 hits)"
grep -rn --color=never '\bsf::' . || echo "  (none)"

heading "Files containing 'SFML_' macros (should be 0 hits)"
grep -rn --color=never '\bSFML_[A-Z]' . || echo "  (none)"

heading "Files containing 'SFML::' (should be 0 hits)"
grep -rn --color=never '\bSFML::' . || echo "  (none)"

heading "Lingering '<SFML/' or '\"SFML/' includes (should be 0 hits)"
grep -rn --color=never '#\s*include\s*[<"]SFML/' . || echo "  (none)"

heading "Substring 'sfml' / 'SFML' anywhere -- needs HUMAN judgement"
echo "  (string literals, comments, doc references, variable names, ...)"
grep -rin --color=never 'sfml' . || echo "  (none)"

heading "Files containing 'Zancle' or 'ZancleBase' (sanity: should be > 0)"
grep -rln --color=never 'Zancle' . | head -30
