#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "shell/shell.h"
#include "drivers/io.h"
#include "memory/gdt.h"
#include "cpu/interrupts.h"
#include <stdint.h>

void kernel_main(uint32_t mb_addr) {
    terminal_init();
    init_GDT();
    init_IDT();
    picremap();
    init_keyboard();
    enable_interrupts();
    shell();
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}