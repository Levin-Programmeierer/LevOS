#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_FLAG_MEMORY  (1 << 0)
#define MULTIBOOT_FLAG_MMAP    (1 << 6)

typedef struct
{
    uint32_t flags;

    uint32_t mem_lower;
    uint32_t mem_upper;

    uint32_t boot_device;

    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    uint8_t syms[16];

    uint32_t mmap_length;
    uint32_t mmap_addr;

    uint32_t drives_length;
    uint32_t drives_addr;

    uint32_t config_table;

    uint32_t boot_loader_name;

    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;

    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

} multiboot_info_t;

typedef struct
{
    uint32_t size;

    uint32_t base_low;
    uint32_t base_high;

    uint32_t length_low;
    uint32_t length_high;

    uint32_t type;

} multiboot_mmap_entry_t;

#endif