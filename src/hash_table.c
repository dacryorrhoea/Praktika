#include "utils.h"
#include "hash_table.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

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

void hash_table_insert(
    HashTable *table, wchar_t symb, wchar_t pair_symb
) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *node = malloc(sizeof(HashNode));
    node->symbol = symb;
    node->count = 0;
    node->pair_symbol = pair_symb;
    node->next = table->buckets[i];
    table->buckets[i] = node;
}

int hash_table_increment(HashTable *table, wchar_t symb) {
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

HashNode* hash_table_return(HashTable *table, wchar_t symb) {
    int i = get_unicode_hash(symb, table->size);
    HashNode *curr = table->buckets[i];
    while (curr) {
        if (curr->symbol == symb) {
            return curr;
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