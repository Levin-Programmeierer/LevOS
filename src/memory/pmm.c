#include <stdint.h>

extern unsigned int kernel_end;

uintptr_t next_free_page = (uintptr_t)&kernel_end;


unsigned int allocate_page(void){
    unsigned int current_page;
    current_page = next_free_page;
    next_free_page = next_free_page + 4096; // Advance to next page
    return current_page;
}