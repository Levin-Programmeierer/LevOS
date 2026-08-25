#include "syscalls.h"
#include "drivers/terminal.h"
#include "process/process.h"

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

void syscall_handler(unsigned int number, unsigned int arg1, unsigned int arg2) {
    if (number == SYS_WRITE) {

        char *text = (char *)arg1;

        for (unsigned int i = 0; i < arg2; i++) {
            putchar(text[i]);
        }

        return;
    }

    if (number == SYS_EXIT) {

        process_exit();

        return;
    }

    print("Unknown syscall: ", RED);
    print_hex_dword(number);
    print("\n", WHITE);
}