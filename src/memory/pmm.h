#ifndef MEMORY_PMM_H
#define MEMORY_PMM_H

#include <stdint.h>

void pmm_init(uint32_t multiboot_info_address);
uint32_t allocate_page(void);
void free_page(uint32_t address);
uint32_t pmm_total_pages(void);
uint32_t pmm_free_pages(void);

#endif
