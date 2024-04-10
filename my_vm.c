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
outer_level_table* directory;

/* Variables */
// Number of physical and virtual pages
size_t num_physical_pages;
size_t num_virtual_pages;
size_t num_physical_bytes;
size_t num_virtual_bytes;

// Number of bits per level
size_t outer_level_bits;
size_t inner_level_bits;
size_t page_offset;

/* Sets a bit given a bitmap */
void set_bit(char* bitmap, unsigned int page_number) {
    page_number -= 1;
    unsigned int byte_index = page_number / 8;
    unsigned int bit_position = page_number % 8;
    bitmap[byte_index] |= (1 << bit_position);
}

/* sets the bit split for the pages */
void set_split() {
    printf("SET_SPLIT()\n");
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

/* Initializes directory AKA outer level table */
void initialize_tables() {
    printf("INITIALIZE_TABLES()\n");

    // initialize the directory at the start of physical memory or page 1
    directory = (outer_level_table*)physical_mem;
    directory->num_of_entries = 1 << outer_level_bits;

    // setting bit to 1
    set_bit(physical_bitmap, 1);

    // initialize the entries array right after the directory
    directory->entries = (inner_level_table**)(physical_mem + sizeof(outer_level_table));

    // create a position pointer that first points to where the first inner table is going to be placed on page 2
    char* current_position = (char*)physical_mem + PAGE_SIZE;
    
    printf("start of physical mem: %p\n", physical_mem);
    printf("first inner table on page 2: %p\n", current_position);

    // add inner page tables starting from page 2
    printf("Allocating inner tables\n");
    for (int i = 0; i < directory->num_of_entries; i++) {
        printf("Inner table %d at %p\n", i+1, current_position);
        // inner level table points to the current position
        directory->entries[i] = (inner_level_table*)current_position;
        // setting bit to 1
        set_bit(physical_bitmap, i+2);
        // initializing number of entries in each table
        directory->entries[i]->num_of_entries = 1 << inner_level_bits;
        // calculating starting position for the inner table entries
        page_table_entry* entries_start = (page_table_entry*)(current_position + sizeof(inner_level_table));
        // connecting inner level table entries and entries_start location
        directory->entries[i]->entries = entries_start;
        for (int j = 0; j < (1 << inner_level_bits); j++) {
            entries_start[j].present = 0;
        }
        current_position += PAGE_SIZE;
    }
}

/* Uses the physical or virtual bitmap to find the next available physical page */
/* Returns the page number */
unsigned int find_free_page(char* bitmap, size_t bitmap_size) {
    printf("FIND_FREE_PAGE()\n");
    for (int i = 0; i < bitmap_size / 8; i++) {
        printf("iterating thru byte %zu - bits %zu to %zu\n", i+1, i*8+1, i*8+8);
        if (bitmap[i] != (char)0xFF) {
            printf("FOUND in byte %zu!\n", i+1);
            for (int j = 0; j < 8; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    printf("Page %zu is free!\n", i*8 + j + 1);
                    return i * 8 + j + 1;
                }
            }
        }
    }
    return 0;
}

void set_physical_mem() {
    printf("SET_PHYSICAL_MEM\n");
    // Allocating physical memory
    physical_mem = malloc(MEMSIZE);
    if (physical_mem == NULL) {
        perror("physical memory allocation failed!\n");
    }
    // Initializing bitmaps - Need to divide by 8 since malloc uses bytes instead of bits
    num_physical_bytes = num_physical_pages / 8;
    physical_bitmap = (char*)malloc(num_physical_bytes);
    if (physical_bitmap == NULL) {
        perror("physical bitmap allocation failed!\n");
    }
    num_virtual_bytes = num_virtual_pages / 8;
    virtual_bitmap = (char*)malloc(num_virtual_bytes);
    if (virtual_bitmap == NULL) {
        perror("virtual bitmap allocation failed!\n");
    }
    // Setting allocated memory to 0
    memset(physical_bitmap, 0, num_physical_bytes);
    memset(virtual_bitmap, 0, num_virtual_bytes);
    // Setting split
    set_split();
    initialize_tables();

    printf("Number of pages in physical memory: %zu\n", num_physical_pages);
    printf("Number of pages in virtual memory: %zu\n", num_virtual_pages);

    printf("Number of bytes in physical bitmap: %zu\n", num_physical_bytes);
    printf("Number of bytes in virtual bitmap: %zu\n", num_virtual_bytes);

    printf("Page size: %ld\n", PAGE_SIZE);
}

void* translate(unsigned int vp){
    printf("TRANSLATE()\n");
    /*
    TO DO:
    1. isolate pdi, pti, and offset bits
    2. access the page directory entry using pdi
    3. access the inner level table entry using pti
    4. calculate the physical address
    */
   // page directory index bit ops
   unsigned int pdi = (vp >> (inner_level_bits + page_offset));
   // page table index bit ops
   unsigned int pti = vp >> page_offset;
   unsigned int pti_mask = (1 << inner_level_bits) - 1;
   pti = pti & pti_mask;
   // page offset bit ops
   unsigned int offset;
   unsigned int offset_mask = (1 << page_offset) - 1;
   offset = vp & offset_mask;
   // print statements
   printf("pdi: %d\n", pdi);
   printf("pti: %d\n", pti);
   printf("page offset: %d\n", offset);

   // Check the page directory
    if (directory->entries[pdi] == NULL) {
        printf("page fault!\n");
        return NULL;
    }
   // Check the inner level table
   inner_level_table* table = directory->entries[pdi];
   // Multiplies Page no. * Page size to get the right memory location
   if (table->entries[pti].present == 0) {
        printf("Entry does not exist!\n");
        return NULL;
   }
   unsigned int physical_page_address = table->entries[pti].page * PAGE_SIZE;
   // Start of physical memory
   char* physical_mem_base = (char*)physical_mem;
   // Start of physical memory + number of pages + page offset
   void* physical_address = (void*)(physical_mem_base + physical_page_address + offset);

   printf("Translated virtual address %u to physical address %p\n", vp, (void*)physical_address);

   return physical_address;
}

unsigned int page_map(unsigned int vp) {
    printf("PAGE_MAP()\n");

   // page directory index bit ops
   unsigned int pdi = (vp >> inner_level_bits);

   // page table index bit ops
   unsigned int pti = vp & ((1 << inner_level_bits) - 1);

   // print statements
   printf("pdi: %d\n", pdi);
   printf("pti: %d\n", pti);
    // check if directory maps to the inner level table

    if (!directory->entries[pdi]) {
        printf("page fault!\n");
        return -1;
    }
    inner_level_table* table = directory->entries[pdi];

    // if mapping is not present
    if (table->entries[pti].present != 1) {
        unsigned int free_page = find_free_page(physical_bitmap, num_physical_pages);
        if (free_page == (unsigned int)-1) {
            printf("No free physical pages available\n");
            return -1;
        }
        set_bit(physical_bitmap, free_page);
        table->entries[pti].present = 1;
        table->entries[pti].page = free_page;

        printf("Mapped virtual page %d to physical page %d\n", vp, free_page);
        return 0;
    } else {
        printf("Mapping already exists for virtual page %d\n", vp);
        return -1;
    }
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
    printf("MAT MULT\n");
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
