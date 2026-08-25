#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#include "shell/shell.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "process/process.h"
#include "drivers/io.h"
#include "cpu/timer.h"

extern unsigned char _binary_build_user_bin_start[];
extern unsigned char _binary_build_user_bin_end[];

extern void load_idt(void);
extern void enable_paging(void);
extern void picremap(void);

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

void kernel_main(void) {
    terminal_init();
    print("Initialising GDT\n", WHITE);
    initialise_GDT();
    print("Initialising IDT\n", WHITE);
    initialise_IDT();
    print("Remapping PIC\n", WHITE);
    picremap();
    print("Loading IDT\n", WHITE);
    load_idt();
    enable_interrupts();
    print("Initialising Keyboard\n", YELLOW);
    init_keyboard();
    print("Initialising timer\n", WHITE);
    timer_init(100);
    print("\nStarting shell\n", GREEN);
    clear();
    shell();
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