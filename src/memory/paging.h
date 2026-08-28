#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#include <stdint.h>

void init_paging(void);
void load_page_directory(uint32_t *directory);
void enable_paging(void);
void map_page(uint32_t virtual_address, uint32_t physical_address,
              unsigned char mode);
void map_page_in(uint32_t *directory, uint32_t virtual_address,
                 uint32_t physical_address, unsigned char mode);
uint32_t *create_page_directory(void);
void reload_page_directory(void);
int user_range_valid(uint32_t *directory, uint32_t address, uint32_t length);

extern uint32_t page_directory[1024];

#endif
