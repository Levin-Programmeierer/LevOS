#ifndef PAGING_H
#define PAGING_H

void init_paging(void);
void load_page_directory(unsigned int *directory);
void enable_paging(void);
void map_page(unsigned int virtual_address, unsigned int physical_address);
void reload_page_directory(void);

extern unsigned int page_directory[1024];
extern unsigned int page_table[1024];

#endif