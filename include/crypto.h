#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>

char* encryption(char* filename);
char* decryption_with_key(char* shifr_filename, char* key_filename);
char* decryption_without_key(char* shifr_filename);

#endif
