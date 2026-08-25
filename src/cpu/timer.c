#include "timer.h"
#include "drivers/io.h"
#include "drivers/terminal.h"

static unsigned int ticks = 0;

void timer_init(unsigned int frequency) {
    unsigned int divisor = 1193180 / frequency;

    print("PIT divisor: ");
    print_hex_dword(divisor);
    print("\n");

    outb(0x43, 0x36);

    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}
void timer_handler(void) {
    ticks++;

    if ((ticks % 100) == 0) {
        print(".");
    }
}

unsigned int timer_ticks(void) {
    return ticks;
}