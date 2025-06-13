#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <wchar.h>

typedef struct HashTable HashTable;

int get_unicode_hash(wchar_t symb, int table_size);

HashTable *create_hash_table(int size);
void hash_table_insert(HashTable *table, wchar_t symb);

int hash_table_contains_and_increment(HashTable *table, wchar_t symb);
size_t hash_table_contains_and_return_count(HashTable *table, wchar_t symb);

void free_hash_table(HashTable *table);

#endif
