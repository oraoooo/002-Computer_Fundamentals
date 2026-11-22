#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

out=$(printf "echo hello world | tr a-z A-Z\nexit\n" | ./main)
if echo "$out" | grep -q "HELLO WORLD"; then
    echo "PASS: shell pipeline (cmd1 | cmd2) works"
else
    echo "FAIL: shell pipeline did not produce expected output"
    echo "$out"
    exit 1
fi
