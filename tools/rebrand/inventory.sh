#!/usr/bin/env bash
# Count every "old" and "new" naming pattern in $1 (defaults to sandbox/).
# Run before and after migrate.sh to verify nothing got mangled or eaten.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET="${1:-sandbox}"
cd "$ROOT/$TARGET"

count() {
    local label="$1" pattern="$2"
    printf "  %-30s %6d\n" "$label" \
        "$(grep -rohE "$pattern" . 2>/dev/null | wc -l)"
}

echo "[inventory of $TARGET/]"
echo
echo "Old patterns (should be 0 post-migration):"
count "sf::base::"    '\bsf::base::'
count "sf::"          '\bsf::'
count "SFML_BASE_"    '\bSFML_BASE_[A-Z_]'
count "SFML_ (other)" '\bSFML_[A-Z]'
count "SFML::"        '\bSFML::'
count "sfml-"         '\bsfml-[a-z]'
count "<SFML/Base/"   '#\s*include\s*[<"]SFML/Base/'
count "<SFML/ (other)" '#\s*include\s*[<"]SFML/[A-Z]'
echo
echo "New patterns (should be > 0 post-migration):"
count "za::"          '\bza::'
count "zb::"          '\bzb::'
count "ZA_"           '\bZA_[A-Z]'
count "ZB_"           '\bZB_[A-Z]'
count "Zancle::"      '\bZancle::'
count "zancle-"       '\bzancle-[a-z]'
count "<Zancle/"      '#\s*include\s*[<"]Zancle/'
count "<ZancleBase/"  '#\s*include\s*[<"]ZancleBase/'
echo
echo "Suspicious survivors (case-insensitive 'sfml' anywhere -- review):"
grep -rohi 'sfml[a-z_]*' . 2>/dev/null | sort -u | sed 's/^/    /'
