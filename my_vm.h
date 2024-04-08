#include <stddef.h>

#define MAX_MEMSIZE (1ULL<<32) // Virtual Memory (2^32 = 4 GB)
#define MEMSIZE (1UL<<30) // Physical Memory (2^30 = 1 GB)
#define PAGE_SIZE (1UL<<13) // Page Size (2^13 = 8 KB)
#define TLB_ENTRIES 256 // Translation Lookup Buffer entries?

typedef struct {
    unsigned int present: 1; // 0 or 1 depending on whether the page is present in memory
    unsigned int frame: 31; // the physical page that corresponds with the virtual address
} page_table_entry;

typedef struct {
    unsigned int num_of_entries; // number of entries in the inner page table
    page_table_entry* entries; // the array of entries
} inner_level_table;

typedef struct {
    unsigned int num_of_entries;
    inner_level_table** entries;
} outer_level_table;

void set_physical_mem();

void initialize_tables();

void * translate(unsigned int vp);

unsigned int page_map(unsigned int vp);

void * t_malloc(size_t n);

int t_free(unsigned int vp, size_t n);

int put_value(unsigned int vp, void *val, size_t n);

int get_value(unsigned int vp, void *dst, size_t n);

void mat_mult(unsigned int a, unsigned int b, unsigned int c, size_t l, size_t m, size_t n);

void add_TLB(unsigned int vpage, unsigned int ppage);

int check_TLB(unsigned int vpage);

void print_TLB_missrate();
