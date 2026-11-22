# writeAHashTableInC

Based on James Routley's [Write a Hash Table in C](https://github.com/jamesroutley/write-a-hash-table) —
open addressing + double hashing, resize kalo load factor lewat 70%.

## Yang gue tambahin: chaining variant + benchmark

`hash_table_chain.c/.h` itu hash table versi separate chaining, API-nya
sama (key/value), cuma cara ngatasin collision-nya beda. `benchmark.c`
nge-insert/search dua-duanya di beberapa ukuran (1rb sampe 100rb key) terus
print perbandingan waktunya.

## Nemu bug beneran, bukan cuma angka benchmark

Pas dijalanin dengan data gede (bukan contoh 2-item dari tutorial),
`ht_insert` malah stuck. Abis di-debug, ternyata di `ht_get_hash`: step
double-hashing-nya itu `hash_b + 1`. Ukuran tabelnya selalu prime (biar
semua step di `[1, size-1]` otomatis coprime sama size), TAPI kalo pas
`hash_b == size - 1`, step-nya jadi `size`, alias **0 mod size** — tiap
attempt nge-probe slot yang sama persis, selamanya, kalo slot itu udah
kepake key lain. Fix-nya di `hash_table.c`: clamp step-nya ke
`[1, num_buckets-1]` pake `(hash_b % (num_buckets - 1)) + 1`, jadi gak akan
pernah 0.

Hasil benchmark-nya sendiri: chaining tahan load factor ~200% (gak perlu
resize) dan tetep lebih cepet per-operasi dibanding open addressing yang
dijaga di bawah 70% pake resize berkala — tradeoff waktu vs memory yang
klasik, sekarang ada angkanya, bukan cuma kata orang.

## Jalanin

```sh
make hashtable && ./build/hash_table      # contoh dasar dari tutorial
make benchmark && ./build/benchmark       # perbandingan oa vs chaining
```

## Test

```sh
make test
```

Ngecek contoh dasar tutorial masih jalan, terus jalanin benchmark —
sekaligus regression-test buat fix double-hashing-nya (kalo masih ada
bug-nya, ini bakal hang bukannya exit normal).
