#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


// error message macro
#define DIE(msg) do { \
    fprintf(stderr, "[%s:%d] %s: ", __FILE__, __LINE__, (msg)); \
    perror(""); \
    exit(EXIT_FAILURE); \
} while(0)


void *xmalloc(size_t len, size_t size);

void **alloc_matrix(size_t rows, size_t cols, size_t size);

void free_matrix(void *matrix, size_t rows);

#endif