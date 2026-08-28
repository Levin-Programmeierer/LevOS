#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "shell/shell.h"
#include "drivers/io.h"
#include "memory/gdt.h"
#include "memory/pmm.h"
#include "memory/paging.h"
#include "cpu/interrupts.h"
#include "cpu/timer.h"
#include "process/process.h"
#include "process/scheduler.h"
#include "process/process.h"
#include <stdint.h>

void enable_interrupts(void);
int user_test_create(void);

void kernel_main(uint32_t multiboot_magic, uint32_t mb_addr) {
    terminal_init();
    init_GDT();
    if (multiboot_magic == 0x36d76289)
        pmm_init(mb_addr);
    else
        pmm_init(0);
    init_paging();
    init_IDT();
    picremap();
    timer_init(100);
    scheduler_init();
    user_test_create();
    shell_filesystem_init();
    init_keyboard();
    enable_interrupts();
    shell();
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}