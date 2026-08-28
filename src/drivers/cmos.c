#include "drivers/cmos.h"
#include "drivers/io.h"

void select_cmos_reg(unsigned char reg)
{
    outb(0x70, reg);
}

unsigned char read_cmos(void)
{
    return inb(0x71);
}
