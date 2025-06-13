#include "utils.h"
#include "crypto.h"
#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <math.h>

typedef struct {
    wchar_t letter;
    float frequency;
} LetterFreq;


void *read_letters_freq_from_bin(const char *filename, size_t *out_count) {
    if (!out_count) return NULL;

    FILE *file = fopen(filename, "rb");
    if (!file) DIE("Failed to open file for writing");

    size_t count;
    fread(&count, sizeof(size_t), 1, file);

    LetterFreq *letter_freq_arr = (LetterFreq *)xmalloc(count, sizeof(LetterFreq));
    fread(letter_freq_arr, sizeof(LetterFreq), count, file);
    
    fclose(file);

    *out_count = count;

    return letter_freq_arr;
}


void decryption_without_key() {
    // якобы выбрали russian в меню
    // 
    // read letter frequencies
    size_t alphabet_length = 0;
    LetterFreq *reference_frequency = (LetterFreq *)read_letters_freq_from_bin(
        "./data/frequency/russian_frequency.bin", &alphabet_length
    );


    HashTable *hash_tbl_counter = (HashTable *)create_hash_table(200);

    for (size_t i = 0; i < alphabet_length; i++) {
        hash_table_insert(hash_tbl_counter, reference_frequency[i].letter);
        hash_table_insert(hash_tbl_counter, reference_frequency[i].letter + 32);
    };


    // counting symbols in cryption text
    FILE *file = fopen("./data/shifr.txt", "rb");
    if (!file) DIE("some troubles with files");

    int curr_symb = 0;
    while ((curr_symb = fgetwc(file)) != EOF) {
        hash_table_contains_and_increment(hash_tbl_counter, curr_symb);
    }

    fclose(file);


    // calculation of pearson's square
    LetterFreq *source_frequency = (LetterFreq *)xmalloc(
        alphabet_length, sizeof(LetterFreq)
    );

    size_t full_count = 0;
    for (size_t i = 0; i < alphabet_length; i++) {
        size_t count = 0;
        count += hash_table_contains_and_return_count(
            hash_tbl_counter, reference_frequency[i].letter
        );
        count += hash_table_contains_and_return_count(
            hash_tbl_counter, reference_frequency[i].letter + 32
        );

        full_count += count;

        source_frequency[i].letter = reference_frequency[i].letter;
        source_frequency[i].frequency = (float)count;
    };

    for (size_t i = 0; i < alphabet_length; i++) {
        source_frequency[i].frequency /= full_count;
        source_frequency[i].frequency *= 100;
    };    


    // euclidean distance square
    float euclidean_distance_square = 0;
    for (size_t i = 0; i < alphabet_length; i++) {
        euclidean_distance_square += pow(
            source_frequency[i].frequency / 100 - reference_frequency[i].frequency / 100, 2
        ); 
    };

    // Percentage of similarity according to Euclidean distance.
    wprintf(L"%.2f%%", (1 - euclidean_distance_square / 2) * 100);

    for (size_t i = 0; i < 32; i++) {
        for (size_t j = 0; j < 31; j++) {
            if (source_frequency[j].frequency < source_frequency[j + 1].frequency) {
                float temp = source_frequency[j].frequency;
                source_frequency[j].frequency = source_frequency[j + 1].frequency;
                source_frequency[j + 1].frequency = temp;
                
                wchar_t temp1 = source_frequency[j].letter;
                source_frequency[j].letter = source_frequency[j + 1].letter;
                source_frequency[j + 1].letter = temp1;
            }
        }
    }

    
    // key variants
    wchar_t **key_variants = (wchar_t **)alloc_matrix(alphabet_length, 5, sizeof(wchar_t));

    // 1 column
    for (size_t j = 0; j < 5; j++) {
        key_variants[0][j] = source_frequency[j].letter;
    }

    //  n column
    for (size_t j = 0; j < 5; j++) {
        key_variants[alphabet_length - 1][j] = source_frequency[alphabet_length - 1 - j].letter;
    }

    // 2 column
    key_variants[1][0] = source_frequency[1].letter;
    int k = 0;
    for (size_t j = 1; j < 5; j++) {
        if (k == 1) k++;
        key_variants[1][j] = source_frequency[k].letter;
        k++;
    }

    // n - 1 column
    key_variants[alphabet_length - 2][0] = source_frequency[alphabet_length - 2].letter;
    k = alphabet_length - 1;
    for (size_t j = 1; j < 5; j++) {
        if (k == (int)alphabet_length - 2) k--;
        key_variants[alphabet_length - 2][j] = source_frequency[k].letter;
        k--;
    }
    
    for (int i = 2; i < (int)alphabet_length - 2; i++) {
        int shift = 0;
        for (int j = 0; j < 5; j++) {
            key_variants[i][j] = source_frequency[i + shift].letter;
            if (shift >= 0) shift++;
            shift *= -1;
        }
    }
    

    wprintf(L"\n");
    for (size_t i = 0; i < alphabet_length; i++) {
        wprintf(L"%lc --- %.2lf", reference_frequency[i].letter, reference_frequency[i].frequency);
        wprintf(L" | %lc --- %.2lf\n", source_frequency[i].letter, source_frequency[i].frequency);
    }


    for (size_t j = 0; j < 5; j++) {
        for (size_t i = 0; i < alphabet_length; i++) {
            wprintf(L"%lc ", key_variants[i][j]);
        }
        wprintf(L"\n");
    }


    // write key to file
    // FILE *tempe = fopen("./data/key1.txt", "wb");
    // if (!tempe) DIE("some troubles with files");
    // for (size_t j = 0; j < alphabet_length; j++) {
    //     fputwc(reference_frequency[j].letter, tempe);
    // }
    // fputwc('\n', tempe);
    // for (size_t j = 0; j < alphabet_length; j++) {
    //     fputwc(source_frequency[j].letter, tempe);
    // }
    // fclose(tempe);

    // decryption_with_key(32);


    // pearson correlation
    float sum_x = 0.0, sum_y = 0.0, sum_x2 = 0.0, sum_y2 = 0.0, sum_xy = 0.0;

    for (size_t i = 0; i < alphabet_length; ++i) {
        sum_x += source_frequency[i].frequency;
        sum_y += reference_frequency[i].frequency;
        sum_x2 += pow(source_frequency[i].frequency, 2);
        sum_y2 += pow(reference_frequency[i].frequency, 2);
        sum_xy += source_frequency[i].frequency * reference_frequency[i].frequency;
    }

    float numerator = sum_xy - (sum_x * sum_y) / alphabet_length;
    float denominator = sqrt((sum_x2 - (sum_x * sum_x) / alphabet_length) *
                             (sum_y2 - (sum_y * sum_y) / alphabet_length));

    if (!denominator) DIE("err");
    wprintf(L"\n%.2f%%", numerator / denominator * 100);


    free_hash_table(hash_tbl_counter);

    free(source_frequency);
    free(reference_frequency);
    free_matrix(key_variants, 2);
}
