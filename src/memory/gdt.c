#include "memory/gdt.h"
#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr gdtr;

extern void gdt_flush(const struct gdt_ptr *pointer);
extern uint32_t kernel_stack_top;

struct task_state_segment {
    uint32_t previous;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct task_state_segment tss;
static uint8_t tss_stack[4096] __attribute__((aligned(16)));

static void set_entry(unsigned int index, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity)
{
    gdt[index].base_low = (uint16_t)(base & 0xffff);
    gdt[index].base_middle = (uint8_t)((base >> 16) & 0xff);
    gdt[index].base_high = (uint8_t)((base >> 24) & 0xff);
    gdt[index].limit_low = (uint16_t)(limit & 0xffff);
    gdt[index].granularity = (uint8_t)((limit >> 16) & 0x0f);
    gdt[index].granularity |= granularity & 0xf0;
    gdt[index].access = access;
}

void init_GDT(void)
{
    set_entry(0, 0, 0, 0, 0);
    set_entry(1, 0, 0xffffffff, 0x9a, 0xcf); /* ring 0 code */
    set_entry(2, 0, 0xffffffff, 0x92, 0xcf); /* ring 0 data */
    set_entry(3, 0, 0xffffffff, 0xfa, 0xcf); /* ring 3 code */
    set_entry(4, 0, 0xffffffff, 0xf2, 0xcf); /* ring 3 data */
    tss = (struct task_state_segment){0};
    tss.esp0 = (uint32_t)(tss_stack + sizeof(tss_stack));
    tss.ss0 = 0x10;
    tss.iomap_base = sizeof(tss);
    set_entry(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

    gdtr.limit = (uint16_t)(sizeof(gdt) - 1);
    gdtr.base = (uint32_t)gdt;
    gdt_flush(&gdtr);
}

void tss_set_kernel_stack(uint32_t stack_top)
{
    tss.esp0 = stack_top;
}
