#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#include "shell/shell.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "process/process.h"
#include "drivers/io.h"
#include "cpu/timer.h"
#include "process/scheduler.h"
#include <stdint.h>
#include "drivers/pci.h"
#include "drivers/qemu_vga.h"


extern unsigned char _binary_build_user_bin_start[];
extern unsigned char _binary_build_user_bin_end[];

extern void load_idt(void);
extern void enable_paging(void);
extern void picremap(void);
void enable_interrupts(void);

extern unsigned char LIGHT_BLUE;
extern unsigned char LIGHT_CYAN;
extern unsigned char LIGHT_GREEN;
extern unsigned char LIGHT_RED;
extern unsigned char LIGHT_PURPLE;
extern unsigned char WHITE;
extern unsigned char RED;
extern unsigned char BLUE;
extern unsigned char GREEN;
extern unsigned char CYAN;
extern unsigned char BROWN;
extern unsigned char PURPLE;
extern unsigned char YELLOW;
extern unsigned char DARK_GRAY;
extern unsigned char GRAY;
void kernel_main(){
    terminal_init();
    initialise_GDT();
    initialise_IDT();
    picremap();
    load_idt();

    pmm_init();
    init_paging();
    enable_paging();

    process_init();
    scheduler_init();
    timer_init(100);

    unsigned int user_size =
        _binary_build_user_bin_end -
        _binary_build_user_bin_start;

    int pid1 = process_create(
        _binary_build_user_bin_start,
        user_size
    );

    int pid2 = process_create(
        _binary_build_user_bin_start,
        user_size
    );

    if (pid1 < 0 || pid2 < 0) {
        print("Failed to create test processes\n", RED);
        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    print("PID 1: ", WHITE);
    print_hex_dword((unsigned int)pid1);
    putchar('\n');

    print("PID 2: ", WHITE);
    print_hex_dword((unsigned int)pid2);
    putchar('\n');

    enable_interrupts();

    for (;;) {
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