#ifndef HASH_TABLE_CHAIN_H
#define HASH_TABLE_CHAIN_H

// hash table pake separate chaining, buat dibandingin sama punya tutorial
// (open addressing + double hashing di hash_table.h/.c). key/value shape
// sama, hash family sama, cuma beda cara ngatasin collision-nya - liat
// benchmark.c buat perbandingan beneran pas kena load.

typedef struct ht_chain_node {
    char* key;
    char* value;
    struct ht_chain_node* next;
} ht_chain_node;

typedef struct {
    int size;
    int count;
    ht_chain_node** buckets;
} ht_chain_table;

ht_chain_table* htc_new(int size);
void htc_del(ht_chain_table* ht);
void htc_insert(ht_chain_table* ht, const char* key, const char* value);
char* htc_search(ht_chain_table* ht, const char* key);
void htc_delete(ht_chain_table* ht, const char* key);

#endif
