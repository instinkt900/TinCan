#!/usr/bin/env bash
# Launch TinCan in a temporary directory, let it run, close it the way a user would, and check the result.
#
# Usage: tools/smoke_launch.sh [seconds to run, default 5]
#
# Passes (exit 0) when the app exits with code 0 and the log has no [warning] or [error] lines. The Debug
# build enables the Vulkan validation layers, so this is also the check that catches renderer regressions.
# On failure the log is kept and its path is printed.
#
# Needs an X11 display, xdotool and python3.

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/build/Debug/tincan"
RUN_SECONDS="${1:-5}"
WINDOW_NAME="TinCan"

# Two Vulkan validation errors are logged during shutdown by moth_graphics, which destroys its buffers and
# the default image's sampler before the shaders whose destructor frees the descriptor sets pointing at
# them. They are cosmetic and happen after the last frame. Allowed by VUID so that nothing else can hide
# behind the allowance. Remove this once moth_toolkit T-002 is released and TinCan picks up the fix.
ALLOWED_VUIDS='VUID-vkDestroyBuffer-buffer-00922|VUID-vkDestroySampler-sampler-01082'

fail_setup() {
    echo "SMOKE LAUNCH CANNOT RUN: $1"
    exit 2
}

[ -x "$BIN" ] || fail_setup "$BIN not found. Build first."
[ -n "${DISPLAY:-}" ] || fail_setup "DISPLAY is not set."
command -v xdotool > /dev/null || fail_setup "xdotool is not installed."
command -v python3 > /dev/null || fail_setup "python3 is not installed."

# The app writes imgui.ini to the current directory, and reads assets/, data/ and tiled_project/ from it.
# Run it somewhere else, with those linked in, so the user's files are not changed.
DIR="$(mktemp -d -t tincan_smoke.XXXXXX)"
LOG="$DIR/run.log"
for asset_dir in assets data tiled_project; do
    [ -d "$ROOT/$asset_dir" ] || fail_setup "$ROOT/$asset_dir not found."
    ln -s "$ROOT/$asset_dir" "$DIR/$asset_dir" || fail_setup "cannot link $asset_dir into $DIR"
done
cd "$DIR" || fail_setup "cannot enter $DIR"

"$BIN" > "$LOG" 2>&1 &
PID=$!

FAILURES=()

WID="$(timeout 15 xdotool search --sync --pid "$PID" --name "$WINDOW_NAME" 2> /dev/null | head -n 1)"
CLOSE_SENT=0
if [ -z "$WID" ]; then
    FAILURES+=("window did not appear within 15 seconds")
else
    sleep "$RUN_SECONDS"
    if ! kill -0 "$PID" 2> /dev/null; then
        FAILURES+=("app exited before it was closed")
    elif ! python3 "$ROOT/tools/close_window.py" "$WID"; then
        FAILURES+=("could not send the close request")
    else
        CLOSE_SENT=1
    fi
fi

for i in $(seq 1 150); do
    kill -0 "$PID" 2> /dev/null || break
    # A close request sent while the window manager is still taking the window over can be dropped.
    # Send it once more before giving up.
    if [ "$i" -eq 30 ] && [ "$CLOSE_SENT" -eq 1 ]; then
        python3 "$ROOT/tools/close_window.py" "$WID" > /dev/null 2>&1
    fi
    sleep 0.1
done
if kill -0 "$PID" 2> /dev/null; then
    FAILURES+=("app did not exit within 15 seconds of the close request")
    kill -9 "$PID" 2> /dev/null
fi
wait "$PID"
EXIT_CODE=$?

[ "$EXIT_CODE" -eq 0 ] || FAILURES+=("exit code $EXIT_CODE")

# A validation message spans several lines, and the VUID is on a continuation line rather than on the
# line carrying the level, so the log is read as records: one record per timestamped line plus the lines
# that follow it. A warning or error record is unexpected unless it names an allowed VUID.
UNEXPECTED="$(awk -v allowed="$ALLOWED_VUIDS" '
    function flush() {
        if (interesting && (record !~ allowed)) print header
        interesting = 0
    }
    /^\[[0-9][0-9][0-9][0-9]-/ {
        flush()
        header = $0
        record = $0
        interesting = (header ~ /\[(warning|error)\]/)
        next
    }
    { record = record "\n" $0 }
    END { flush() }
' "$LOG")"
if [ -n "$UNEXPECTED" ]; then
    FAILURES+=("log has warning or error lines")
fi

if [ "${#FAILURES[@]}" -eq 0 ]; then
    echo "SMOKE LAUNCH PASSED"
    rm -rf "$DIR"
    exit 0
fi

echo "SMOKE LAUNCH FAILED"
for reason in "${FAILURES[@]}"; do
    echo "  - $reason"
done
echo "Warning and error lines, ignoring the allowed shutdown VUIDs:"
if [ -n "$UNEXPECTED" ]; then
    echo "$UNEXPECTED" | sed 's/^/  /'
else
    echo "  (none)"
fi
echo "Full log: $LOG"
exit 1
