#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#include "shell/shell.h"
#include "memory/paging.h"
#include "memory/pmm.h"
extern unsigned int page_directory[1024];
extern unsigned int page_table[1024];
extern void load_idt(void);
extern void enable_paging(void);

void kernel_main(void) {
	print("Initialising GDT...\n\0");
	initialise_GDT();
	print("Initialising IDT...\n\0");
	initialise_IDT();
	picremap();
	load_idt();
	init_keyboard();
	init_paging();
	enable_paging();

	unsigned int page1 = allocate_page();

	map_page(0x00200000, page1);
	map_page(0x00400000, page1);

	reload_page_directory();

	volatile unsigned int *test =
		(volatile unsigned int *)0x00400000;

	volatile unsigned int *physical =
		(volatile unsigned int *)page1;

	*test = 0x12345678;

	print("\nVirtual: ");
	print_hex_dword(*test);

	print("\nPhysical: ");
	print_hex_dword(*physical);

	print("LevShell 0.1\n\n\0");
	print("> \0");

	__asm__ volatile ("sti");
	shell();
}