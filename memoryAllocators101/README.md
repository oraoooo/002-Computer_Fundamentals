# memoryAllocators101

`malloc`/`free`/`calloc`/`realloc` sendiri, pake `sbrk()` buat minta memory
ke OS. Awalnya dibuat buat di-`LD_PRELOAD` ke program lain (Linux doang
yang reliable).

## Yang gue tambahin: telemetry + leak report

Tiap panggilan malloc/calloc/realloc/free dicatet ke file log (`op size
ptr`), terus pas program exit ada `atexit` hook yang ngecek blok mana yang
masih kepake tapi gak pernah di-free — mini `valgrind --leak-check`
versi sendiri.

Kenapa loggingnya pake `write(2)` langsung, bukan `fprintf`? Soalnya
`FILE*` butuh buffer yang dialokasiin lewat `malloc()` — yang notabene
allocator ini sendiri — jadi bisa deadlock di `global_malloc_lock` kalo
dipaksa pake stdio. Raw fd + stack buffer (`snprintf` ke `buf`, baru
`write`) ga pernah alokasi, jadi aman.

`LD_PRELOAD` agak unreliable di macOS, makanya `test_malloc.c` link
langsung ke `memalloc.c` tanpa preload — allocator yang sama, jalan sama
persis di Linux maupun macOS. Sengaja ninggalin satu blok 16-byte ke-leak.

## Jalanin

```sh
make                                          # bikin memalloc.so + memalloc_test
MEMALLOC_LOG=/tmp/m.log ./memalloc_test       # liat leak report + log-nya
cat /tmp/m.log
```

Linux: `LD_PRELOAD=$PWD/memalloc.so <program apapun>`.

## Test

```sh
make test
```

Jalanin `memalloc_test`, cek report-nya bener-bener bilang
`leaked_blocks=1 leaked_bytes=16`.
