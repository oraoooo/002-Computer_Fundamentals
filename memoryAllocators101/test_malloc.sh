#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

LOG=$(mktemp)
OUT=$(mktemp)
trap 'rm -f "$LOG" "$OUT"' EXIT

MEMALLOC_LOG="$LOG" ./memalloc_test > "$OUT" 2>&1

if grep -q "leaked_blocks=1 leaked_bytes=16" "$OUT"; then
    echo "PASS: leak report found the intentional 1-block/16-byte leak"
else
    echo "FAIL: leak report did not match the expected leak"
    cat "$OUT"
    exit 1
fi

if [ -s "$LOG" ] && grep -q "malloc(new)" "$LOG"; then
    echo "PASS: per-call allocation log was written"
else
    echo "FAIL: allocation log missing or empty"
    exit 1
fi
