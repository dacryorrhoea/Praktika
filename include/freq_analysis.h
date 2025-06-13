#ifndef FREQ_ANALYSIS_H
#define FREQ_ANALYSIS_H

#include <stddef.h>

typedef struct LetterFreq LetterFreq;

void *read_letters_freq_from_bin(const char *filename, size_t *out_count);

void decryption_without_key();

#endif