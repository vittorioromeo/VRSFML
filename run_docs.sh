#!/bin/bash
#
# Regenerate the Doxygen API docs in the active build directory, serve
# the generated HTML over a local HTTP server, and open it in Chromium.
#
# Usage:
#   ./run_docs.sh                # default port 8000
#   PORT=9001 ./run_docs.sh      # custom port
#
# Behavior:
#   - Detects the build directory from $PWD (must be run from a build_*
#     directory, same convention as run_test.sh).
#   - Re-runs `cmake -DZA_BUILD_DOC=ON .` so the doc target is wired up
#     even if you previously configured with the default (OFF).
#   - Builds the `doc` ninja target.
#   - Serves build_*/doc/html via `python3 -m http.server`.
#   - Opens Chromium at the index page; falls back to xdg-open if not
#     present.
#   - Press Ctrl-C to stop; the HTTP server is cleaned up automatically.

set -u

script_dir="$(cd "$(dirname "$0")" && pwd)"
basepwd="$(basename "$PWD")"

if [[ "$basepwd" != build_* ]]; then
    echo "run_docs.sh: must be invoked from a build_* directory (cd build_vrdev_clang first)" >&2
    exit 1
fi

PORT="${PORT:-8000}"

echo "=== Configuring with -DZA_BUILD_DOC=ON ==="
cmake -DZA_BUILD_DOC=ON . >/dev/null || { echo "cmake configure failed" >&2; exit 1; }

echo "=== Building doc target ==="
ninja doc || { echo "doc build failed" >&2; exit 1; }

html_dir="$PWD/doc/html"
if [ ! -f "$html_dir/index.html" ]; then
    echo "run_docs.sh: $html_dir/index.html not found -- did the build succeed?" >&2
    exit 1
fi

# Auto-bump the port if the requested one is busy; up to 10 tries.
for _ in 1 2 3 4 5 6 7 8 9 10; do
    if ! ss -ltn "sport = :$PORT" 2>/dev/null | grep -q LISTEN; then
        break
    fi
    PORT=$((PORT + 1))
done

echo "=== Serving $html_dir on http://localhost:$PORT ==="

# Start Python HTTP server in the html dir. Use exec via a subshell so
# the trap below can kill it cleanly.
(cd "$html_dir" && exec python3 -m http.server "$PORT" >/dev/null 2>&1) &
server_pid=$!

# Ensure the server is shut down on Ctrl-C / script exit.
trap 'kill "$server_pid" 2>/dev/null; wait "$server_pid" 2>/dev/null; exit' INT TERM EXIT

# Give the server a moment to bind.
sleep 0.3

url="http://localhost:$PORT/index.html"

# Prefer chromium; fall back to xdg-open. Background so the script
# keeps running and the trap still controls the lifetime.
if command -v chromium >/dev/null 2>&1; then
    chromium "$url" >/dev/null 2>&1 &
elif command -v chromium-browser >/dev/null 2>&1; then
    chromium-browser "$url" >/dev/null 2>&1 &
elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$url" >/dev/null 2>&1 &
else
    echo "run_docs.sh: no chromium/xdg-open found -- visit $url manually" >&2
fi

echo "=== Press Ctrl-C to stop the server ==="
wait "$server_pid"
