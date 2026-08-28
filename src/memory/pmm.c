#include "memory/pmm.h"

#define PAGE_SIZE       4096u
#define MAX_PAGES       (1024u * 1024u) /* all of the 32-bit address space */
#define BITMAP_BYTES    (MAX_PAGES / 8u)

extern unsigned int kernel_end;

/* A bit set means that the corresponding physical page is unavailable. */
static uint8_t page_bitmap[BITMAP_BYTES] __attribute__((aligned(16)));
static uint32_t highest_page;
static uint32_t free_count;

static void set_used(uint32_t page)
{
    page_bitmap[page >> 3] |= (uint8_t)(1u << (page & 7));
}

static void set_free(uint32_t page)
{
    page_bitmap[page >> 3] &= (uint8_t)~(1u << (page & 7));
}

static int is_used(uint32_t page)
{
    return (page_bitmap[page >> 3] & (1u << (page & 7))) != 0;
}

static void free_range(uint64_t start, uint64_t end)
{
    uint64_t first = (start + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last = end / PAGE_SIZE;
    if (last > MAX_PAGES)
        last = MAX_PAGES;
    for (uint64_t page = first; page < last; ++page) {
        if (is_used((uint32_t)page)) {
            set_free((uint32_t)page);
            ++free_count;
        }
    }
    if (last > highest_page)
        highest_page = (uint32_t)last;
}

static void reserve_range(uint64_t start, uint64_t end)
{
    uint64_t first = start / PAGE_SIZE;
    uint64_t last = (end + PAGE_SIZE - 1) / PAGE_SIZE;
    if (first >= MAX_PAGES)
        return;
    if (last > MAX_PAGES)
        last = MAX_PAGES;
    for (uint64_t page = first; page < last; ++page) {
        if (!is_used((uint32_t)page)) {
            set_used((uint32_t)page);
            --free_count;
        }
    }
}

void pmm_init(uint32_t multiboot_info_address)
{
    for (uint32_t i = 0; i < BITMAP_BYTES; ++i)
        page_bitmap[i] = 0xff;
    highest_page = 0;
    free_count = 0;

    if (multiboot_info_address != 0) {
        uint32_t total_size = *(uint32_t *)(uintptr_t)multiboot_info_address;
        uint32_t offset = 8;
        while (offset + 8 <= total_size) {
            uint32_t *tag = (uint32_t *)(uintptr_t)(multiboot_info_address + offset);
            uint32_t type = tag[0];
            uint32_t size = tag[1];
            if (size < 8 || offset + size > total_size)
                break;
            if (type == 6) {
                /* memory map entries start after the tag's size/entry fields */
                uint32_t entry_size = tag[2];
                if (entry_size >= 24) {
                    uint32_t entries = (size - 16) / entry_size;
                    uint8_t *entry = (uint8_t *)tag + 16;
                    for (uint32_t i = 0; i < entries; ++i) {
                        uint64_t base = *(uint64_t *)(entry + 0);
                        uint64_t length = *(uint64_t *)(entry + 8);
                        uint32_t kind = *(uint32_t *)(entry + 16);
                        if (kind == 1 && length != 0)
                            free_range(base, base + length);
                        entry += entry_size;
                    }
                }
            }
            offset += (size + 7) & ~7u;
        }
        reserve_range(multiboot_info_address,
                      (uint64_t)multiboot_info_address + total_size);
    }

    /* A boot without a usable map still gets a small, useful heap. */
    if (highest_page == 0)
        free_range(0x00100000, 0x01000000);

    reserve_range(0, 0x1000);
    reserve_range(0, (uint64_t)(uintptr_t)&kernel_end);
}

uint32_t allocate_page(void)
{
    for (uint32_t page = 1; page < highest_page; ++page) {
        if (!is_used(page)) {
            set_used(page);
            --free_count;
            return page * PAGE_SIZE;
        }
    }
    return 0;
}

void free_page(uint32_t address)
{
    uint32_t page = address / PAGE_SIZE;
    if (page == 0 || page >= highest_page || !is_used(page))
        return;
    set_free(page);
    ++free_count;
}

uint32_t pmm_total_pages(void) { return highest_page; }
uint32_t pmm_free_pages(void) { return free_count; }
