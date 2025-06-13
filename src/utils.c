#include "utils.h"

#include <stdio.h>
#include <stdlib.h>


void *xmalloc(size_t len, size_t size) {
    void *ptr = malloc(len * size);
    if (!ptr) DIE("xmalloc: failed");
    return ptr;
}


void **alloc_matrix(size_t rows, size_t cols, size_t size) {
    void **matrix = malloc(rows * sizeof(void *));
    if (!matrix) DIE("alloc_matrix: row allocation failed");

    for (size_t i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * size);
        if (!matrix[i]) {
            for (size_t j = 0; j < i; j++)
                free(matrix[j]);
            free(matrix);
            DIE("alloc_matrix: column allocation failed");
        }
    }

    return matrix;
}


void free_matrix(void *matrix, size_t rows) {
    if (!matrix) return;
    void **mtrx = (void **)matrix;
    for (size_t i = 0; i < rows; i++)
        free(mtrx[i]);
    free(mtrx);
}