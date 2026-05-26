#!/usr/bin/env bash
# Reset sandbox/ to a fresh copy of baseline/. Run after every migration
# experiment so the next run starts from the same starting point.
#
# Sandbox-only: this script assumes a layout like
#     <root>/baseline/    pristine source corpus
#     <root>/sandbox/     mutable working copy
#     <root>/scripts/     where this script lives
# For the real repo migration you don't need reset.sh -- use `git restore .`
# or branch off again.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

rm -rf sandbox
cp -a baseline sandbox
echo "sandbox/ reset from baseline/"
