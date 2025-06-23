#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <wchar.h>

typedef struct HashNode {
    wchar_t symbol;
    wchar_t pair_symbol;
    size_t count;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode **buckets;
    size_t size;
} HashTable;

HashTable *create_hash_table(int size);
void hash_table_insert(HashTable *table, wchar_t symb, wchar_t pair_symb);
int hash_table_increment(HashTable *table, wchar_t symb);
HashNode* hash_table_return(HashTable *table, wchar_t symb);
void free_hash_table(HashTable *table);

#endif
