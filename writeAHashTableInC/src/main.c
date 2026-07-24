#include <stdio.h>
#include "hash_table.h"

int main(){
    ht_hash_table* ht = ht_new();
    
    ht_insert(ht, "name", "Alice");
    ht_insert(ht, "age", "30");
    
    printf("name: %s\n", ht_search(ht, "name"));
    printf("age: %s\n", ht_search(ht, "age"));
    
    ht_delete(ht, "name");
    printf("name after delete: %s\n", ht_search(ht, "name"));
    
    ht_del_hash_table(ht);
    return 0;
}