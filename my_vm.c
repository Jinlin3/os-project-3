/*
Name: Jack Lin
Net ID: jjl327
Username of iLab: kill.cs.rutgers.edu
*/

#include "my_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Structures
void* physical_mem;
char* physical_bitmap;
char* virtual_bitmap;
outer_level_table* directory;
tlb_entry tlb[TLB_ENTRIES];

/* Variables */
// Number of physical and virtual pages
size_t num_physical_pages;
size_t num_virtual_pages;
size_t num_physical_bytes;
size_t num_virtual_bytes;
int tlb_entry_count = 0;
int tlb_check_count = 0;
int tlb_miss_count = 0;

// Number of bits per level
size_t outer_level_bits;
size_t inner_level_bits;
size_t page_offset;

/* Sets a bit given a bitmap; pages are 1-indexed */
void set_bit(char* bitmap, unsigned int page_number) {
    page_number -= 1;
    unsigned int byte_index = page_number / 8;
    unsigned int bit_position = page_number % 8;
    bitmap[byte_index] |= (1 << bit_position);
}

void unset_bit(char* bitmap, unsigned int page_number) {
    page_number -= 1;
    unsigned int byte_index = page_number / 8;
    unsigned int bit_position = page_number % 8;
    bitmap[byte_index] &= ~(1 << bit_position);
}

/* determines split for outer-level table, inner-level table, and page offset */
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
}

/* Initializes directory AKA outer level table + TLB */
void initialize_tables() {
    // initialize the directory at the start of physical memory or page 1
    directory = (outer_level_table*)physical_mem;
    directory->num_of_entries = 1 << outer_level_bits;

    // setting bit to 1
    set_bit(physical_bitmap, 1);

    // initialize the entries array right after the directory
    directory->entries = (inner_level_table**)(physical_mem + sizeof(outer_level_table));

    // create a position pointer that first points to where the first inner table is going to be placed on page 2
    char* current_position = (char*)physical_mem + PAGE_SIZE;

    // add inner page tables starting from page 2
    for (int i = 0; i < directory->num_of_entries; i++) {
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
    // initialize TLB
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb[i].present = 0;
    }
}

/* Uses the physical or virtual bitmap to find the next available physical page */
/* Returns the page number */
unsigned int find_free_page(char* bitmap, size_t bitmap_size) {
    for (int i = 0; i < bitmap_size / 8; i++) {
        if (bitmap[i] != (char)0xFF) {
            for (int j = 0; j < 8; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    return i * 8 + j + 1;
                }
            }
        }
    }
    return (unsigned int)-1;
}

void set_physical_mem() {
    // Analyzing virtual and physical memory bits
    set_split();

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

    // Initializing page tables and entries
    initialize_tables();
}

void* translate(unsigned int vp){
   // page directory index bit ops
   unsigned int pdi = (vp >> (inner_level_bits + page_offset));
   // page table index bit ops
   unsigned int pti_mask = (1 << inner_level_bits) - 1;
   unsigned int pti = (vp >> page_offset) & pti_mask;
   // offset bit ops
   unsigned int offset_mask = (1 << page_offset) - 1;
   unsigned int offset = vp & offset_mask;

   // Start of physical memory
   char* physical_mem_base = (char*)physical_mem;

   // Check the TLB first
   int ppn = check_TLB((vp / PAGE_SIZE) + 1);
   if (ppn != -1) {
        return (void*)(physical_mem_base + (ppn * PAGE_SIZE) + offset);
   }

   // Check the page directory
    if (directory->entries[pdi] == NULL) {
        perror("page fault!\n");
        return NULL;
    }
   // Check the inner level table
   inner_level_table* table = directory->entries[pdi];
   // Multiplies Page no. * Page size to get the right memory location
   if (table->entries[pti].present == 0) {
        return NULL;
   }
   unsigned int physical_page_address = table->entries[pti].page * PAGE_SIZE;
   // Start of physical memory + number of pages + page offset
   void* physical_address = (void*)(physical_mem_base + physical_page_address + offset);

   // Add entries to TLB
   add_TLB((vp / PAGE_SIZE) + 1, table->entries[pti].page);

   return physical_address;
}

unsigned int page_map(unsigned int vp) {
   // page directory index bit ops
   unsigned int pdi = (vp >> (inner_level_bits + page_offset));
   // page table index bit ops
   unsigned int pti_mask = (1 << inner_level_bits) - 1;
   unsigned int pti = (vp >> page_offset) & pti_mask;
   // offset bit ops
   unsigned int offset_mask = (1 << page_offset) - 1;
   unsigned int offset = vp & offset_mask;

    // check if directory maps to the inner level table
    if (!directory->entries[pdi]) {
        perror("page fault!\n");
        return (unsigned int)-1;
    }
    inner_level_table* table = directory->entries[pdi];

    // if mapping is not present
    if (table->entries[pti].present != (unsigned int)1) {
        unsigned int free_page = find_free_page(physical_bitmap, num_physical_pages);
        if (free_page == (unsigned int)-1) {
            perror("No free physical pages available\n");
            return (unsigned int)-1;
        }
        set_bit(physical_bitmap, free_page);
        table->entries[pti].present = (unsigned int)1;
        table->entries[pti].page = free_page;

        return free_page;
    } else {
        return table->entries[pti].page;
    }
}

void* t_malloc(size_t n) {
   // Calculating required number of pages
   size_t num_of_pages = (n + PAGE_SIZE - 1) / PAGE_SIZE;
   
   unsigned int first_virtual_address = 0; // virtual address we will return
   unsigned int virtual_address;
   unsigned int free_virtual_page;
   // Finding free physical and virtual pages
   for (int i = 0; i < num_of_pages; i++) {
        // checking if there are free pages in physical memory
        if (find_free_page(physical_bitmap, num_physical_pages) == -1) {
            return NULL;
        }
        // find a free virtual page
        free_virtual_page = find_free_page(virtual_bitmap, num_virtual_pages);
        /*
        1. Convert virtual page number to a virtual address
        2. Use page_map to map the virtual address to a physical address
        3. Update bitmap
        */
        if (i == 0) {
            first_virtual_address = ((free_virtual_page - 1) * PAGE_SIZE);
            page_map(first_virtual_address);
            set_bit(virtual_bitmap, free_virtual_page);
        } else {
            virtual_address = ((free_virtual_page - 1) * PAGE_SIZE);
            page_map(virtual_address);
            set_bit(virtual_bitmap, free_virtual_page);
        }
   }
   return (void*)first_virtual_address;
}

int t_free(unsigned int vp, size_t n) {
    // Calculating starting address and page using vp
    unsigned int virtual_page = (vp / PAGE_SIZE) + 1;
    unsigned int virtual_address = (virtual_page - 1) * PAGE_SIZE;
    unsigned int physical_page;

    for (int i = 0; i < n; i++) {
        if (!translate(virtual_address)) {
            return -1;
        } else {
            // page directory index bit ops
            unsigned int pdi = (virtual_address >> (inner_level_bits + page_offset));
            // page table index bit ops
            unsigned int pti_mask = (1 << inner_level_bits) - 1;
            unsigned int pti = (virtual_address >> page_offset) & pti_mask;
            // offset bit ops
            unsigned int offset_mask = (1 << page_offset) - 1;
            unsigned int offset = virtual_address & offset_mask;

            inner_level_table* table = directory->entries[pdi];
            physical_page = page_map(virtual_address);
            unset_bit(virtual_bitmap, virtual_page);
            unset_bit(physical_bitmap, physical_page);
            table->entries[pti].page = 0;
            table->entries[pti].present = 0;
        }
        virtual_page++;
        virtual_address += PAGE_SIZE;
    }
    return 0;
}

int put_value(unsigned int vp, void *val, size_t n) {
    // Checking val
    if (!val) {
        return -1;
    }
    // Aligning address to the start of the page
    unsigned int page_offset_bits = vp & ((1 << page_offset) - 1);
    unsigned int remaining_size = n;
    // Align to the start of the page
    unsigned int current_virtual_address = vp - page_offset_bits;
    char* value_ptr = (char*)val;

    while (remaining_size > 0) {
        // Translate virtual address to physical address
        void* physical_address = translate(current_virtual_address);
        if (!physical_address) {
            return -1;
        }

        // Determine size to copy for this page
        size_t copy_size = PAGE_SIZE;
        if (copy_size > remaining_size) {
            copy_size = remaining_size;
        }

        // Copy data to physical memory
        memcpy((char*)physical_address + page_offset_bits, value_ptr, copy_size);

        // Update for the next page
        remaining_size -= copy_size;
        value_ptr += copy_size;
        current_virtual_address += PAGE_SIZE;
    }

    return 0;
}

int get_value(unsigned int vp, void *dst, size_t n){
    if (!dst) {
        return -1;
    }
    // Aligning address to the start of the page
    unsigned int page_offset_bits = vp & ((1 << page_offset) - 1);
    unsigned int remaining_size = n;
    // Align to the start of the page
    unsigned int current_virtual_address = vp - page_offset_bits;
    char* dst_ptr = (char*)dst;

    while (remaining_size > 0) {
        // Translate virtual address to physical address
        void* physical_address = translate(current_virtual_address);
        if (!physical_address) {
            return -1;
        }

        // Determine size to copy for this page
        size_t copy_size = PAGE_SIZE;
        if (copy_size > remaining_size) {
            copy_size = remaining_size;
        }

        // Copy data to physical memory
        memcpy(dst_ptr, (char*)physical_address + page_offset_bits, copy_size);

        // Update for the next page
        remaining_size -= copy_size;
        dst_ptr += copy_size;
        current_virtual_address += PAGE_SIZE;
    }
    
    return 0;
}

void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n) {
    for (int i = 0; i < l; i++) { // Rows of Matrix A
        for (int j = 0; j < n; j++) { // Columns of Matrix B
            int sum = 0; // The result of dot product
            for (int k = 0; k < m; k++) { // Dot product loop
                // Virtual addresses for elements of matrices A and B
                unsigned int address_a = a + ((i * m + k) * sizeof(int));
                unsigned int address_b = b + ((k * n + j) * sizeof(int));
                // Getting their values
                int valA = *(int*)translate(address_a);
                int valB = *(int*)translate(address_b);
                sum += valA * valB;
            }
            unsigned int addrC = c + ((i * n + j) * sizeof(int));
            int *resultCell = (int*)translate(addrC);
            *resultCell = sum;
        }
    }
}

void add_TLB(unsigned int vpage, unsigned int ppage) {
    if (tlb_entry_count < TLB_ENTRIES) {
        tlb[tlb_entry_count].vpn = vpage;
        tlb[tlb_entry_count].ppn = ppage;
        tlb[tlb_entry_count].present = 1;
    } else {
        int x = vpage % TLB_ENTRIES;
        tlb[x].vpn = vpage;
        tlb[x].ppn = ppage;
    }
}

int check_TLB(unsigned int vpage) {
    tlb_check_count++;
    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (tlb[i].present == 1 && tlb[i].vpn == vpage) {
            return tlb[i].ppn;
        }
    }
    tlb_miss_count++;
    return -1;
}

void print_TLB_missrate(){
    float tlb_miss_rate = (float)tlb_miss_count / (float)tlb_check_count;
    printf("TLB Miss Rate: %d / %d = %.2f%%\n", tlb_miss_count, tlb_check_count, tlb_miss_rate * 100);
}