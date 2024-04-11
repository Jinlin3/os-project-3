#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initialize_matrices(void* mat1, void* mat2, size_t l, size_t m, size_t n) {
    srand(time(NULL)); // Seed for random number generation
    for (size_t i = 0; i < l; i++) {
        for (size_t j = 0; j < m; j++) {
            int value = rand() % 10; // Random value between 0 and 9
            put_value(mat1 + (i * m + j) * sizeof(int), &value, sizeof(int));
        }
    }

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            int value = rand() % 10; // Random value between 0 and 9
            put_value(mat2 + (i * n + j) * sizeof(int), &value, sizeof(int));
        }
    }
}

void print_matrix(void* mat, size_t rows, size_t cols) {
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            int value;
            get_value(mat + (i * cols + j) * sizeof(int), &value, sizeof(int));
            printf("%d ", value);
        }
        printf("\n");
    }
}

int main() {
    set_physical_mem(); // Initialize your memory management system

    size_t l = 2, m = 2, n = 2; // Example matrix sizes for the tester

    // Allocate memory for matrices
    void* mat1 = t_malloc(l * m * sizeof(int));
    void* mat2 = t_malloc(m * n * sizeof(int));
    void* result = t_malloc(l * n * sizeof(int));

    // Populate matrices with values
    initialize_matrices(mat1, mat2, l, m, n);

    printf("Matrix 1:\n");
    print_matrix(mat1, l, m);
    printf("\nMatrix 2:\n");
    print_matrix(mat2, m, n);

    // Perform matrix multiplication
    mat_mult((unsigned int)mat1, (unsigned int)mat2, (unsigned int)result, l, m, n);

    // Print result matrix
    printf("\nResult Matrix:\n");
    print_matrix(result, l, n);

    // Free allocated memory
    t_free((unsigned int)mat1, l * m * sizeof(int));
    t_free((unsigned int)mat2, m * n * sizeof(int));
    t_free((unsigned int)result, l * n * sizeof(int));

    return 0;
}
