#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO: Define static variables and structs, include headers, etc.

void* physical_mem;
size_t num_physical_pages;
size_t num_virtual_pages;
char* physical_bitmap;
char* virtual_bitmap;

void set_physical_mem() {
    // Allocating physical memory
    physical_mem = malloc(MEMSIZE);
    if (physical_mem == NULL) {
        perror("physical memory allocation failed!\n");
    }

    // Number of pages needed
    num_physical_pages = MEMSIZE / PAGE_SIZE;
    num_virtual_pages = MAX_MEMSIZE / PAGE_SIZE;

    /* Creating bitmaps
        Need to divide by 8 since malloc uses bytes instead of bits
    */
    physical_bitmap = (char*)malloc(num_physical_pages / 8);
    if (physical_bitmap == NULL) {
        perror("physical bitmap allocation failed!\n");
    }
    virtual_bitmap = (char*)malloc(num_virtual_pages / 8);
    if (virtual_bitmap == NULL) {
        perror("virtual bitmap allocation failed!\n");
    }

    // Setting allocated memory to 0
    memset(physical_bitmap, 0, num_physical_pages / 8);
    memset(virtual_bitmap, 0, num_virtual_pages / 8);

    printf("Number of pages in physical memory: %zu\n", num_physical_pages);
    printf("Number of pages in virtual memory: %zu\n", num_virtual_pages);
}

void * translate(unsigned int vp){
    //TODO: Finish
}

unsigned int page_map(unsigned int vp){
    //TODO: Finish
}

void * t_malloc(size_t n){
    //TODO: Finish
}

int t_free(unsigned int vp, size_t n){
    //TODO: Finish
}

int put_value(unsigned int vp, void *val, size_t n){
    //TODO: Finish
}

int get_value(unsigned int vp, void *dst, size_t n){
    //TODO: Finish
}

void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n){
    printf("matrix multiplication!\n");
}

void add_TLB(unsigned int vpage, unsigned int ppage){
    //TODO: Finish
}

int check_TLB(unsigned int vpage){
    //TODO: Finish
}

void print_TLB_missrate(){
    //TODO: Finish
}
