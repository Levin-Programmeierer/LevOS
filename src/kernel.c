#include "terminal.h"
#include "keyboard.h"
#include "interrupts.h"
#include "io.h"
#include "shell.h"

extern void load_idt(void);

void kernel_main(void) {
    print("Initialising GDT...\n\0");
    initialise_GDT();
    print("Initialising IDT...\n\0");
    initialise_IDT();
    print("Remapping PIC...\n\0");
    picremap();
    print("Loading IDT...\n\0");
    load_idt();
    print("Initialising keyboard...\n\0");
    init_keyboard();
    //clear();
    print("LevShell 0.1\n\n\0");
    print("> \0");
    __asm__ volatile ("sti"); // start interrupts
    shell();
}