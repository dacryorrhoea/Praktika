#include "utils.h"

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#include <time.h>


void encryption(size_t alphabet_length, int alphabet_first_char) {
    srand((unsigned) time(NULL));

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


    FILE *src = fopen("./data/voyna_i_mir.txt", "rb");
    FILE *tmp = fopen("./data/shifr.txt", "wb");
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
    tmp = fopen("./data/key.txt", "wb");
    if (!tmp) DIE("some troubles with files");
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < alphabet_length; j++) {
            fputwc(key[i][j], tmp);
        }
        fputwc('\n', tmp);
    }
    fclose(tmp);

    free_matrix(key, 2);
}


void decryption_with_key(size_t alphabet_length) {
    // read key
    int **key = (int **)alloc_matrix(2, alphabet_length, sizeof(int));

    FILE *tmp = fopen("./data/key1.txt", "rb");
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
    

    FILE *src = fopen("./data/shifr.txt", "rb");
    tmp = fopen("./data/temp_deshifr.txt", "wb");
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
}