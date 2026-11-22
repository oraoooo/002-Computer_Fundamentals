// penasaran doang: tabel open-addressing + double-hashing punya tutorial
// ini beneran lebih cepet dari separate chaining, atau cuma lebih hemat
// memory? coba diukur beberapa load factor terus dibandingin.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hash_table.h"
#include "hash_table_chain.h"
#include "prime.h"

static char** make_keys(int n, unsigned seed){
    srand(seed);
    char** keys = malloc((size_t)n * sizeof(char*));
    for (int i = 0; i < n; i++) {
        keys[i] = malloc(32);
        snprintf(keys[i], 32, "key-%06d-%06x", i, rand());
    }
    return keys;
}

static void free_keys(char** keys, int n){
    for (int i = 0; i < n; i++) free(keys[i]);
    free(keys);
}

static double elapsed(clock_t start, clock_t end){
    return (double)(end - start) / CLOCKS_PER_SEC;
}

static void run_trial(int n){
    char** keys = make_keys(n, 42);
    char** miss_keys = make_keys(n, 1337);

    ht_hash_table* oa = ht_new();
    // sengaja dibikin cuma separo n, biar chaining-nya kepaksa load factor
    // ~200% - mau liat seberapa kuat dia nanggung beban tanpa resize
    ht_chain_table* ch = htc_new(next_prime(n < 50 ? 50 : n / 2));

    clock_t t0 = clock();
    for (int i = 0; i < n; i++) ht_insert(oa, keys[i], "v");
    clock_t t1 = clock();
    for (int i = 0; i < n; i++) htc_insert(ch, keys[i], "v");
    clock_t t2 = clock();

    clock_t t3 = clock();
    for (int i = 0; i < n; i++) ht_search(oa, keys[i]);
    clock_t t4 = clock();
    for (int i = 0; i < n; i++) htc_search(ch, keys[i]);
    clock_t t5 = clock();

    clock_t t6 = clock();
    for (int i = 0; i < n; i++) ht_search(oa, miss_keys[i]);
    clock_t t7 = clock();
    for (int i = 0; i < n; i++) htc_search(ch, miss_keys[i]);
    clock_t t8 = clock();

    double oa_load = 100.0 * oa->count / oa->size;
    double ch_load = 100.0 * ch->count / ch->size;

    printf("n=%-7d | insert  oa=%.4fs ch=%.4fs | hit    oa=%.4fs ch=%.4fs | miss   oa=%.4fs ch=%.4fs | load%% oa=%.1f ch=%.1f\n",
        n,
        elapsed(t0, t1), elapsed(t1, t2),
        elapsed(t3, t4), elapsed(t4, t5),
        elapsed(t6, t7), elapsed(t7, t8),
        oa_load, ch_load);

    ht_del_hash_table(oa);
    htc_del(ch);
    free_keys(keys, n);
    free_keys(miss_keys, n);
}

int main(void){
    printf("open-addressing (oa, double hashing, resizing) vs separate chaining (ch, fixed size)\n\n");
    int sizes[] = {1000, 5000, 20000, 100000};
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        run_trial(sizes[i]);
    }
    return 0;
}
