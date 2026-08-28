#include "drivers/cmos.h"
#include "drivers/io.h"

void select_cmos_reg(unsigned char reg){
    outb(0x70, reg);
}

unsigned char read_cmos(void){
    return inb(0x71);
}

unsigned short get_cmos_memory_size(void) {
    unsigned short total;
    unsigned char lowmem, highmem;

    outb(0x70, 0x30);
    lowmem = inb(0x71);
    outb(0x70, 0x31);
    highmem = inb(0x71);

    total = lowmem | highmem << 8;
    return total;
}
