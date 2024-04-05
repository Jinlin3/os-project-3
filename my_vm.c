#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//TODO: Define static variables and structs, include headers, etc.

// Structures
void* physical_mem;
char* physical_bitmap;
char* virtual_bitmap;

/* Variables */
// Number of physical and virtual pages
size_t num_physical_pages;
size_t num_virtual_pages;

// Number of bits per level
size_t outer_level_bits;
size_t inner_level_bits;
size_t page_offset;

/* sets the bit split for the pages */
void set_split() {
    // Number of pages needed
    num_physical_pages = MEMSIZE / PAGE_SIZE;
    num_virtual_pages = MAX_MEMSIZE / PAGE_SIZE;
    // Setting number of bits for each
    int bits_left = 32;
    page_offset = (int)log2(PAGE_SIZE);
    bits_left -= page_offset;
    if (bits_left % 2 != 0) {
        inner_level_bits = (bits_left / 2) + 1;
    } else {
        inner_level_bits = bits_left / 2;
    }
    bits_left -= inner_level_bits;
    outer_level_bits = bits_left;
    
    // Print statements for testing split
    printf("page offset: %d\n", page_offset);
    printf("inner level bits: %d\n", inner_level_bits);
    printf("outer level bits: %d\n", outer_level_bits);
}

void set_physical_mem() {
    // Allocating physical memory
    physical_mem = malloc(MEMSIZE);
    if (physical_mem == NULL) {
        perror("physical memory allocation failed!\n");
    }
    // Setting split
    set_split();
    // Initializing bitmaps - Need to divide by 8 since malloc uses bytes instead of bits
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
    printf("Page size: %ld\n", PAGE_SIZE);
    printf("Size of outer table: %d\n", sizeof(outer_level_table));
    printf("Size of inner table: %d\n", sizeof(inner_level_table));
    printf("Size of page table entry: %d\n", sizeof(page_table_entry));

    // Initializing outer level page table
    /*
        TODO:
        1. Determine split between outer and inner level table
        2. Check space for 
    */
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
