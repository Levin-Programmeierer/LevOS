#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#include "shell/graphicsshell.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "process/process.h"
#include "drivers/io.h"
#include "cpu/timer.h"
#include "process/scheduler.h"
#include "drivers/framebuffer_driver.h"
#include <stdint.h>


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
/*
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

    print("Initialising Keyboard\n", YELLOW);
    init_keyboard();

    print("Initialising Timer\n", WHITE);
    timer_init(100);

    print("Initialising PMM\n", WHITE);
    pmm_init();

    print("Initialising Paging\n", WHITE);
    init_paging();

    enable_paging();

    print("Initialising Processes\n", WHITE);
    process_init();

    print("Initialising Scheduler\n", WHITE);
    scheduler_init();

    unsigned int user_size =
        _binary_build_user_bin_end -
        _binary_build_user_bin_start;

    print("Creating process 1...\n", WHITE);

    int pid1 = process_create(
        _binary_build_user_bin_start,
        user_size
    );

    if (pid1 < 0) {
        print("Process 1 creation FAILED\n", RED);

        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    print("Creating process 2...\n", WHITE);

    int pid2 = process_create(
        _binary_build_user_bin_start,
        user_size
    );

    if (pid2 < 0) {
        print("Process 2 creation FAILED\n", RED);

        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    print("PID 1: ", WHITE);
    print_hex_dword((unsigned int)pid1);
    print("\n", WHITE);

    print("PID 2: ", WHITE);
    print_hex_dword((unsigned int)pid2);
    print("\n", WHITE);

    print("Starting PID 1...\n", WHITE);

    process_run(pid1);

    print("PID 1 returned!\n", WHITE);

    enable_interrupts();

    clear();

    shell();
}
Testing kernel*/

void kernel_main(uint32_t mbi_addr){
    framebuffer_init(mbi_addr);
    font_init();
    initialise_GDT();
    initialise_IDT();
    picremap();
    load_idt();
    init_keyboard();
    enable_interrupts();

    graphicsshell();
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