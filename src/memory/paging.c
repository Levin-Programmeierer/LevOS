#include "memory/paging.h"
#include "memory/pmm.h"

#define PRESENT 0x001u
#define WRITABLE 0x002u
#define USER 0x004u

uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t identity_tables[4][1024] __attribute__((aligned(4096)));

extern void load_page_directory(uint32_t *directory);
extern void enable_paging(void);

static uint32_t flags_for(unsigned char mode)
{
    return (mode == 'U' || mode == 'u') ? (PRESENT | WRITABLE | USER)
                                         : (PRESENT | WRITABLE);
}

static int user_page_present(uint32_t *directory, uint32_t address)
{
    uint32_t directory_entry = directory[(address >> 22) & 0x3ff];
    if ((directory_entry & (PRESENT | USER)) != (PRESENT | USER))
        return 0;

    uint32_t *table =
        (uint32_t *)(uintptr_t)(directory_entry & 0xfffff000u);
    uint32_t page_entry = table[(address >> 12) & 0x3ff];
    return (page_entry & (PRESENT | USER)) == (PRESENT | USER);
}

void init_paging(void)
{
    for (uint32_t i = 0; i < 1024; ++i)
        page_directory[i] = 0;

    /* Identity mapping keeps the kernel, VGA memory, stacks and PMM pages
       directly addressable while the kernel is still a low-memory image. */
    for (uint32_t table = 0; table < 4; ++table) {
        for (uint32_t entry = 0; entry < 1024; ++entry)
            identity_tables[table][entry] =
                ((table * 1024u + entry) * 0x1000u) | PRESENT | WRITABLE;
        page_directory[table] =
            (uint32_t)identity_tables[table] | PRESENT | WRITABLE;
    }

    /* Recursive mapping is useful to future callers and makes the directory
       layout conventional, while all current kernel addresses stay identity
       mapped. */
    page_directory[1023] = (uint32_t)page_directory | PRESENT | WRITABLE;
    load_page_directory(page_directory);
    enable_paging();
}

void map_page_in(uint32_t *directory, uint32_t virtual_address,
                 uint32_t physical_address, unsigned char mode)
{
    uint32_t di = (virtual_address >> 22) & 0x3ff;
    uint32_t ti = (virtual_address >> 12) & 0x3ff;
    uint32_t flags = flags_for(mode);
    uint32_t *table;

    if (directory[di] & PRESENT) {
        table = (uint32_t *)(directory[di] & 0xfffff000u);
        if (mode == 'U' || mode == 'u')
            directory[di] |= USER;
    } else {
        uint32_t table_address = allocate_page();
        if (table_address == 0)
            return;
        table = (uint32_t *)(uintptr_t)table_address;
        for (uint32_t i = 0; i < 1024; ++i)
            table[i] = 0;
        directory[di] = table_address | flags;
    }
    table[ti] = (physical_address & 0xfffff000u) | flags;
    if (directory == page_directory)
        __asm__ volatile ("invlpg (%0)" : : "r"(virtual_address) : "memory");
}

void map_page(uint32_t virtual_address, uint32_t physical_address,
              unsigned char mode)
{
    map_page_in(page_directory, virtual_address, physical_address, mode);
}

uint32_t *create_page_directory(void)
{
    uint32_t address = allocate_page();
    if (address == 0)
        return (uint32_t *)0;
    uint32_t *directory = (uint32_t *)(uintptr_t)address;
    for (uint32_t i = 0; i < 1024; ++i)
        directory[i] = page_directory[i];
    directory[1023] = address | PRESENT | WRITABLE;
    return directory;
}

void reload_page_directory(void)
{
    load_page_directory(page_directory);
}

int user_range_valid(uint32_t *directory, uint32_t address, uint32_t length)
{
    const uint32_t user_limit = 0xc0000000u;
    if (directory == 0 || address >= user_limit)
        return 0;
    if (length == 0)
        return 1;
    if (length > user_limit - address)
        return 0;

    uint32_t end = address + length - 1;
    uint32_t last_page = end & 0xfffff000u;
    for (uint32_t page = address & 0xfffff000u;; page += 0x1000u) {
        if (!user_page_present(directory, page))
            return 0;
        if (page == last_page)
            break;
    }
    return 1;
}
