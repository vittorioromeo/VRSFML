#!/usr/bin/env bash
# Serve the landing page locally and open it in Chromium.
# Use Ctrl+C to stop both the server and the browser.

set -euo pipefail

PORT=8123
HOST=127.0.0.1

cd "$(dirname "$(readlink -f "$0")")"

# Pick the first chromium-like binary available on the system.
BROWSER=""
for candidate in chromium chromium-browser google-chrome google-chrome-stable chrome; do
    if command -v "$candidate" >/dev/null 2>&1; then
        BROWSER="$candidate"
        break
    fi
done

if [[ -z "$BROWSER" ]]; then
    echo "error: no chromium binary found in PATH" >&2
    echo "  tried: chromium, chromium-browser, google-chrome, google-chrome-stable, chrome" >&2
    exit 1
fi

# Start the server bound to localhost only.
python3 -m http.server "$PORT" --bind "$HOST" &
SERVER_PID=$!
trap 'kill "$SERVER_PID" 2>/dev/null || true' EXIT INT TERM

# Give the server a moment to come up before launching the browser.
sleep 0.3

echo "serving on http://${HOST}:${PORT}/"
echo "opening in $BROWSER"

"$BROWSER" "http://${HOST}:${PORT}/" >/dev/null 2>&1 &

# Block until the server exits (or this script is interrupted).
wait "$SERVER_PID"
