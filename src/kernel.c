#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#include "shell/shell.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "process/process.h"

extern unsigned char _binary_build_user_bin_start[];
extern unsigned char _binary_build_user_bin_end[];

extern void load_idt(void);
extern void enable_paging(void);

void kernel_main(void) {
    print("Initialising GDT...\n");
    initialise_GDT();

    print("Initialising IDT...\n");
    initialise_IDT();

    print("Remapping PIC...\n");
    picremap();

    load_idt();

    print("PIC MASK: ");
    print_hex_byte(inb(0x21));
    print("\n");

    print("Initialising keyboard...\n");
    init_keyboard();

    print("Initialising PIT...\n");
    timer_init(100);

    __asm__ volatile ("sti");

    unsigned int flags;

    __asm__ volatile (
        "pushf\n"
        "pop %0"
        : "=r"(flags)
    );

    print("EFLAGS: ");
    print_hex_dword(flags);
    print("\n");

    print("Waiting for IRQ0...\n");

    while (1) {
        unsigned char irr;
        unsigned char isr;
        unsigned char imr;

        outb(0x20, 0x0A);
        irr = inb(0x20);

        outb(0x20, 0x0B);
        isr = inb(0x20);

        imr = inb(0x21);

        if (irr & 0x01) {
            print("IRR0 ");
            print_hex_byte(irr);

            print(" ISR ");
            print_hex_byte(isr);

            print(" IMR ");
            print_hex_byte(imr);

            print("\n");
        }

        __asm__ volatile ("hlt");
    }
}


//copied from chatgpt
void enter_user_mode(void)
{
    __asm__ volatile (
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "push $0x23\n"
        "push $0x00801000\n"

        "pushf\n"
        "pop %%eax\n"
        "or $0x200, %%eax\n"
        "push %%eax\n"

        "push $0x1B\n"
        "push $0x00400000\n"

        "iret\n"
        :
        :
        : "eax"
    );
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}