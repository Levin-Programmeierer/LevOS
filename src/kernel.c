#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "shell/shell.h"
#include "drivers/io.h"
#include "memory/gdt.h"
#include <stdint.h>

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

void kernel_main(uint32_t mb_addr) {
    init_GDT();
    shell();
}

void enable_interrupts(void) {
    __asm__ volatile ("sti");
}