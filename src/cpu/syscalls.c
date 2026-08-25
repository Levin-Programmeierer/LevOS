#include "syscalls.h"
#include "drivers/terminal.h"
#include "process/process.h"

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

    print("Unknown syscall: ");
    print_hex_dword(number);
    print("\n");
}