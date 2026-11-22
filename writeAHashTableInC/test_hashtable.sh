#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"

out=$(./build/hash_table)
if echo "$out" | grep -q "^name: Alice$" \
   && echo "$out" | grep -q "^age: 30$" \
   && echo "$out" | grep -q "name after delete: (null)"; then
    echo "PASS: hash table basic insert/search/delete"
else
    echo "FAIL: hash table basic ops"
    echo "$out"
    exit 1
fi

# ini juga regression-test buat fix double-hashing di hash_table.c: dulu
# sebelum di-fix, insert kebanyakan bisa bikin ht_insert stuck selamanya di
# satu slot. kalo benchmark-nya selesai (gak hang), berarti fix-nya masih nempel.
./build/benchmark
echo "PASS: benchmark ran to completion at all load factors (double-hashing infinite-loop bug stays fixed)"
