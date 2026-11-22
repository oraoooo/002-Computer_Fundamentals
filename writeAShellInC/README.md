# writeAShellInC

Based on Stephen Brennan's [Write a Shell in C](https://brennan.io/2015/01/16/write-a-shell-in-c/).
Basically a mini bash clone — read a line, split it into words, fork + exec.

## Yang gue tambahin: pipe (`|`)

Tutorialnya cuma sampe `execvp()` satu command doang, belum ada piping. Jadi
`ls | wc -l` bakal gagal di versi aslinya. Ini celah asli di tutorialnya,
bukan latihan yang dia bahas — jadi lumayan buat nambah sendiri.

Caranya: `lsh_split_pipeline` motong-motong array token tiap nemu `"|"`,
ganti jadi `NULL`, terus nyatet mulai dari mana command selanjutnya.
`lsh_launch_pipeline` bikin `N-1` pipe, fork `N` child process, terus tiap
child nyambungin stdin/stdout nya ke pipe yang bener lewat `dup2` sebelum
`execvp`. Parent-nya nutup semua pipe fd dia sendiri terus nunggu semua
child biar ga jadi zombie process.

## Jalanin

```sh
make           # compile, hasilnya binary `main`
./main
> echo hi | tr a-z A-Z
HI
> exit
```

## Test

```sh
make test
```

Nge-pipe `echo hello world | tr a-z A-Z` ke dalam shell-nya terus cek
outputnya ada `HELLO WORLD` apa nggak.
