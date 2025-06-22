#include "utils.h"

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#include <time.h>


char* encryption(char* filename) {
    srand((unsigned) time(NULL));
    size_t alphabet_length = 32;
    int alphabet_first_char = 0x0410;
    char filepath[512];

    //  first step -> keygen
    int **key = (int **)alloc_matrix(2, alphabet_length, sizeof(int));


    int curr_char = alphabet_first_char;
    for (size_t i = 0; i < alphabet_length; i++) {
        key[0][i] = key[1][i] = curr_char;
        curr_char++;
    }

    
    // fisher yates shuffle
    for (size_t i = alphabet_length - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        int tmp = key[1][i];
        key[1][i] = key[1][j];
        key[1][j] = tmp;
    }

    snprintf(filepath, sizeof(filepath), "./data/%s", filename);
    FILE *src = fopen(filepath, "rb");

    snprintf(filepath, sizeof(filepath), "./data/shifr_%s", filename);
    FILE *tmp = fopen(filepath, "wb");

    if (!src || !tmp) DIE("some troubles with files");

    // read character by character, encoding the matching ones
    int curr_symb;
    while ((curr_symb = fgetwc(src)) != EOF) {
        int founded = 0;

        for (size_t i = 0; i < alphabet_length; i++) {
            if (curr_symb == key[0][i]) {
                fputwc(key[1][i], tmp);
                founded = 1;
                break;
            } else if (curr_symb == key[0][i] + 32) {
                fputwc(key[1][i] + 32, tmp);
                founded = 1;
                break;
            }
        }
        
        if (!founded) fputwc(curr_symb, tmp);
    }

    fclose(src);
    fclose(tmp);


    // write key to file
    snprintf(filepath, sizeof(filepath), "./data/key_%s", filename);
    tmp = fopen(filepath, "wb");

    if (!tmp) DIE("some troubles with files");

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < alphabet_length; j++) {
            fputwc(key[i][j], tmp);
        }
        fputwc('\n', tmp);
    }
    fclose(tmp);

    free_matrix(key, 2);

    char* filepath_shifr = (char*)xmalloc(512, sizeof(char));
    snprintf(filepath_shifr, 512 * sizeof(char), "shifr_%s", filename);
    return filepath_shifr;
}


char* decryption_with_key(char* shifr_filename, char* key_filename) {
    // read key
    size_t alphabet_length = 32;
    char filepath[512];
    int **key = (int **)alloc_matrix(2, alphabet_length, sizeof(int));

    snprintf(filepath, sizeof(filepath), "./data/%s", key_filename);
    FILE *tmp = fopen(filepath, "rb");

    if (!tmp) DIE("some troubles with files");

    int curr_symb = 0;
    size_t i = 0, j = 0;

    while ((curr_symb = fgetwc(tmp)) != EOF) {
        if (curr_symb != '\n') {
            key[i][j] = curr_symb; // !then perform a length check!
            j++;
        } else {
            i++;
            j = 0;
        }
    }

    fclose(tmp);
    
    snprintf(filepath, sizeof(filepath), "./data/%s", shifr_filename);
    FILE *src = fopen(filepath, "rb");

    snprintf(filepath, sizeof(filepath), "./data/de%s", shifr_filename);
    tmp = fopen(filepath, "wb");

    if (!src || !tmp) DIE("some troubles with files");

    curr_symb = 0;
    while ((curr_symb = fgetwc(src)) != EOF) {
        int founded = 0;

        for (size_t i = 0; i < alphabet_length; i++) {
            if (curr_symb == key[1][i]) {
                fputwc(key[0][i], tmp);
                founded = 1;
                break;
            } else if (curr_symb == key[1][i] + 32) {
                fputwc(key[0][i] + 32, tmp);
                founded = 1;
                break;
            }
        }
        
        if (!founded) fputwc(curr_symb, tmp);
    }


    fclose(src);
    fclose(tmp);

    free_matrix(key, 2);
    key = NULL;

    char* filepath_deshifr = (char*)xmalloc(512, sizeof(char));
    snprintf(filepath_deshifr, 512 * sizeof(char), "de%s", shifr_filename);
    return filepath_deshifr;
}