#include "cpu/timer.h"
#include "drivers/io.h"
#include "process/scheduler.h"

static volatile uint32_t ticks;

void timer_init(uint32_t frequency)
{
    if (frequency == 0)
        frequency = 100;
    uint32_t divisor = 1193180u / frequency;
    if (divisor == 0)
        divisor = 1;
    outb(0x43, 0x36);
    outb(0x40, (unsigned char)(divisor & 0xff));
    outb(0x40, (unsigned char)((divisor >> 8) & 0xff));

    /* IRQ0 was masked by the PIC setup until the PIT was configured. */
    unsigned char mask = inb(0x21);
    outb(0x21, (unsigned char)(mask & ~1u));
}

struct cpu_context *timer_handler(struct cpu_context *context)
{
    ++ticks;
    return scheduler_tick(context);
}

uint32_t timer_ticks(void)
{
    return ticks;
}
