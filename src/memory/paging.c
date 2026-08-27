#include "memory/paging.h"
#include "drivers/terminal.h"
#include "memory/pmm.h"

unsigned int page_directory[1024]
    __attribute__((aligned(4096)));

unsigned int page_table[1024]
    __attribute__((aligned(4096)));


void init_paging(void)
{
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0;

        /* Identity-map first 4 MiB */
        page_table[i] =
            (i * 4096) | 0x03;
    }

    page_directory[0] =
        (unsigned int)page_table | 0x03; // 0x03 = 00000011 Present and Read/Writable

    load_page_directory(page_directory);
}


void map_page_in(
    unsigned int *directory,
    unsigned int virtual_address,
    unsigned int physical_address,
    unsigned char mode
)
{
    unsigned int directory_index =
        (virtual_address >> 22) & 0x3FF;

    unsigned int table_index =
        (virtual_address >> 12) & 0x3FF;

    unsigned int aligned_address =
        physical_address & 0xFFFFF000;

    unsigned int entry_flags;

    if (mode == 'U') {
        entry_flags = 0x07;
    } else {
        entry_flags = 0x03;
    }

    unsigned int *table;

    if (directory[directory_index] & 0x01) {

        table = (unsigned int *)
            (directory[directory_index] & 0xFFFFF000);

        if (mode == 'U') {
            directory[directory_index] |= 0x04;
        }

    } else {

        unsigned int new_page_table =
            allocate_page();

        table = (unsigned int *)new_page_table;

        for (int i = 0; i < 1024; i++) {
            table[i] = 0;
        }

        directory[directory_index] =
            new_page_table | entry_flags;
    }

    table[table_index] =
        aligned_address | entry_flags;
}


void map_page(
    unsigned int virtual_address,
    unsigned int physical_address,
    unsigned char mode
)
{
    map_page_in(
        page_directory,
        virtual_address,
        physical_address,
        mode
    );
}


unsigned int *create_page_directory(void)
{
    unsigned int physical_address =
        allocate_page();

    unsigned int *directory =
        (unsigned int *)physical_address;

    for (int i = 0; i < 1024; i++) {
        directory[i] = 0;
    }

    for (int i = 0; i < 1024; i++) {

        if (page_directory[i] & 0x01) {
            directory[i] =
                page_directory[i];
        }
    }

    return directory;
}


void reload_page_directory(void)
{
    load_page_directory(page_directory);
}