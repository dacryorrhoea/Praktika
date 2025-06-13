#include "utils.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>


// node for store symbol and counter
typedef struct HashNode {
    wchar_t symbol;
    size_t count;
    struct HashNode *next;
} HashNode;

// hash table
typedef struct {
    HashNode **buckets;
    size_t size;
} HashTable;


int get_unicode_hash(wchar_t symb, int table_size) {
    return (symb * 2654435761) % table_size;
}


HashTable *create_hash_table(int size) {
    HashTable *table = malloc(sizeof(HashTable));
    if(!table) DIE("trubble with alloc for hashTable");

    table->buckets = calloc(size, sizeof(HashNode *));
    if(!table) DIE("trubble with alloc for buckets hashTable");

    table->size = size;

    return table;
}


void hash_table_insert(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *node = malloc(sizeof(HashNode));
    node->symbol = symb;
    node->count = 0;
    node->next = table->buckets[i];
    table->buckets[i] = node;
}


int hash_table_contains_and_increment(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *curr = table->buckets[i];
    while (curr) {
        if (curr->symbol == symb) {
            curr->count++;
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}


size_t hash_table_contains_and_return_count(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *curr = table->buckets[i];
    while (curr) {
        if (curr->symbol == symb) {
            return curr->count;
        }
        curr = curr->next;
    }

    return 0;
}


void free_hash_table(HashTable *table) {
    for (size_t i = 0; i < table -> size; i++) {
        if (table -> buckets[i]) {
            HashNode *curr = table -> buckets[i];
            while (curr) {
                HashNode *tmp = curr;
                curr = curr -> next;
                free(tmp);
            };
        }
    }
    free(table -> buckets);
    free(table);
}