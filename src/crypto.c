#include "utils.h"
#include "hash_table.h"
#include "crypto.h"
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#include <time.h>

#define ALPHABET_LENGTH 32
#define ALPHABET_FIRST_SYMB 0x0410

typedef struct {
    wchar_t letter;
    float frequency;
} LetterFreq;


FILE* open_file(char* prefix, char* filename, char* cmd) {
    char filepath[512];

    snprintf(filepath, sizeof(filepath), prefix, filename);
    FILE* file = fopen(filepath, cmd);

    if (!file) DIE("some troubles with files");

    return file;
}

LetterFreq* read_letters_freq(char *lang) {
    FILE *file = open_file(
        "./data/frequency/%s_frequency.bin", lang, "rb"
    );

    size_t n;
    fread(&n, sizeof(size_t), 1, file);

    LetterFreq* freq = (LetterFreq*)xmalloc(n, sizeof(LetterFreq));
    fread(freq, sizeof(LetterFreq), n, file);
    
    fclose(file);

    return freq;
}

char* encryption(char* filename) {
    wchar_t** key = (wchar_t**)alloc_matrix(
        2, ALPHABET_LENGTH, sizeof(wchar_t)
    );
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        key[0][i] = ALPHABET_FIRST_SYMB + i;
        key[1][i] = ALPHABET_FIRST_SYMB + i;
    }

    srand((unsigned) time(NULL));
    for (int i = ALPHABET_LENGTH - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        wchar_t tmp = key[1][i];
        key[1][i] = key[1][j];
        key[1][j] = tmp;
    }

    
    HashTable *hash_tbl = (HashTable *)create_hash_table(200);
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        hash_table_insert(hash_tbl, key[0][i], key[1][i]);
        hash_table_insert(hash_tbl, key[0][i] + 32, key[1][i] + 32);
    };

    FILE *src = open_file("./data/%s", filename, "rb");
    FILE *tmp = open_file("./data/shifr_%s", filename, "wb");

    wchar_t curr_symb = "";
    while ((curr_symb = fgetwc(src)) != EOF) {
        HashNode* symb_node = hash_table_return(hash_tbl, curr_symb);
        fputwc(symb_node? symb_node->pair_symbol: curr_symb, tmp);
    }

    free_hash_table(hash_tbl);
    fclose(src);
    fclose(tmp);


    tmp = open_file("./data/key_%s", filename, "wb");

    fwprintf(tmp, L"%.*ls\n%.*ls\n", 
        ALPHABET_LENGTH, key[0],
        ALPHABET_LENGTH, key[1]
    );

    fclose(tmp);
    free_matrix(key, 2);


    char* filepath_deshifr = (char*)xmalloc(
        strlen(filename) + 7, sizeof(char)
    );
    sprintf(filepath_deshifr, "shifr_%s", filename);
    return filepath_deshifr;
}


char* decryption_with_key(char* shifr_filename, char* key_filename) {
    wchar_t** key = (wchar_t**)alloc_matrix(
        2, ALPHABET_LENGTH, sizeof(wchar_t)
    );
    FILE *tmp = open_file("./data/%s", key_filename, "rb");

    wchar_t curr_symb;
    int i = 0, j = 0;

    while ((curr_symb = fgetwc(tmp)) != EOF) {
        if (curr_symb != '\n') {
            key[i][j++] = curr_symb;
        } else {
            i++;
            j = 0;
        }
    }

    fclose(tmp);

    HashTable *hash_tbl = (HashTable *)create_hash_table(200);
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        hash_table_insert(hash_tbl, key[1][i], key[0][i]);
        hash_table_insert(hash_tbl, key[1][i] + 32, key[0][i] + 32);
    };
    
    free_matrix(key, 2);


    FILE *src = open_file("./data/%s", shifr_filename, "rb");
    tmp = open_file("./data/de%s", shifr_filename, "wb");

    while ((curr_symb = fgetwc(src)) != EOF) {
        HashNode* symb_node = hash_table_return(hash_tbl, curr_symb);
        fputwc(symb_node? symb_node->pair_symbol: curr_symb, tmp);
    }

    fclose(src);
    fclose(tmp);
    free_hash_table(hash_tbl);


    char* filepath_deshifr = (char*)xmalloc(
        strlen(shifr_filename) + 3, sizeof(char)
    );
    sprintf(filepath_deshifr, "de%s", shifr_filename);
    return filepath_deshifr;
}


void sort_freqs(LetterFreq* src_freq) {
    for (size_t i = 0; i < 32; i++) {
        for (size_t j = 0; j < 31; j++) {
            if (src_freq[j].frequency < src_freq[j + 1].frequency) {
                float temp = src_freq[j].frequency;
                src_freq[j].frequency = src_freq[j + 1].frequency;
                src_freq[j + 1].frequency = temp;
                
                wchar_t temp1 = src_freq[j].letter;
                src_freq[j].letter = src_freq[j + 1].letter;
                src_freq[j + 1].letter = temp1;
            }
        }
    }
}

char* decryption_without_key(char* shifr_filename) {
    LetterFreq *ref_freq = read_letters_freq("russian");

    HashTable *hash_tbl = (HashTable *)create_hash_table(200);
    for (size_t i = 0; i < ALPHABET_LENGTH; i++) {
        hash_table_insert(hash_tbl, ref_freq[i].letter, "");
        hash_table_insert(hash_tbl, ref_freq[i].letter + 32, "");
    };


    FILE *file = open_file("./data/%s", shifr_filename, "rb");

    int curr_symb = 0;
    while ((curr_symb = fgetwc(file)) != EOF) {
        hash_table_increment(hash_tbl, curr_symb);
    }

    fclose(file);


    LetterFreq* src_freq = (LetterFreq*)xmalloc(
        ALPHABET_LENGTH, sizeof(LetterFreq)
    );

    size_t full_count = 0;
    for (size_t i = 0; i < ALPHABET_LENGTH; i++) {
        size_t count = 0;
        count += hash_table_return(hash_tbl, ref_freq[i].letter)->count;
        count += hash_table_return(hash_tbl, ref_freq[i].letter + 32)->count;

        full_count += count;

        src_freq[i].letter = ref_freq[i].letter;
        src_freq[i].frequency = (float)count;
    };

    for (size_t i = 0; i < ALPHABET_LENGTH; i++) {
        src_freq[i].frequency /= full_count;
        src_freq[i].frequency *= 100;
    };

    sort_freqs(src_freq);

    file = open_file("./data/FA_key_%s", shifr_filename, "wb");

    for (size_t j = 0; j < ALPHABET_LENGTH; j++) {
        fputwc(ref_freq[j].letter, file);
    }
    fputwc('\n', file);
    for (size_t j = 0; j < ALPHABET_LENGTH; j++) {
        fputwc(src_freq[j].letter, file);
    }

    fclose(file);

    free_hash_table(hash_tbl);

    free(src_freq);
    free(ref_freq);
    

    char* filepath_deshifr = (char*)xmalloc(
        strlen(shifr_filename) + 8, sizeof(char)
    );
    sprintf(filepath_deshifr, "FA_key_%s", shifr_filename);
    return filepath_deshifr;
}