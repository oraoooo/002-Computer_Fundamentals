#include <stdlib.h>
#include <string.h>

#include "hash_table_chain.h"
#include "hash_table.h"

// hash biasa aja, single hash doang - gak perlu double hashing kaya di
// hash_table.c soalnya collision-nya ditangani pake linked list per bucket
static int htc_hash(const char* s, int m){
    long hash = 0;
    const int len_s = (int)strlen(s);
    for(int i = 0; i < len_s; i++){
        hash = (hash * HT_PRIME_1 + s[i]) % m;
    }
    return (int)hash;
}

ht_chain_table* htc_new(int size){
    ht_chain_table* ht = malloc(sizeof(ht_chain_table));
    ht->size = size;
    ht->count = 0;
    ht->buckets = calloc((size_t)size, sizeof(ht_chain_node*));
    return ht;
}

void htc_del(ht_chain_table* ht){
    for(int i = 0; i < ht->size; i++){
        ht_chain_node* node = ht->buckets[i];
        while(node){
            ht_chain_node* next = node->next;
            free(node->key);
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(ht->buckets);
    free(ht);
}

void htc_insert(ht_chain_table* ht, const char* key, const char* value){
    int index = htc_hash(key, ht->size);
    ht_chain_node* node = ht->buckets[index];
    while(node){
        if(strcmp(node->key, key) == 0){
            free(node->value);
            node->value = strdup(value);
            return;
        }
        node = node->next;
    }
    // ga ketemu, tambahin node baru di depan bucket-nya (gampang, O(1))
    ht_chain_node* new_node = malloc(sizeof(ht_chain_node));
    new_node->key = strdup(key);
    new_node->value = strdup(value);
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    ht->count++;
}

char* htc_search(ht_chain_table* ht, const char* key){
    int index = htc_hash(key, ht->size);
    ht_chain_node* node = ht->buckets[index];
    while(node){
        if(strcmp(node->key, key) == 0){
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void htc_delete(ht_chain_table* ht, const char* key){
    int index = htc_hash(key, ht->size);
    ht_chain_node* node = ht->buckets[index];
    ht_chain_node* prev = NULL;
    while(node){
        if(strcmp(node->key, key) == 0){
            if(prev) prev->next = node->next;
            else ht->buckets[index] = node->next;
            free(node->key);
            free(node->value);
            free(node);
            ht->count--;
            return;
        }
        prev = node;
        node = node->next;
    }
}
