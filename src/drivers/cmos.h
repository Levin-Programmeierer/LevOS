#ifndef DRIVERS_CMOS_H
#define DRIVERS_CMOS_H

void select_cmos_reg(unsigned char reg);
unsigned char read_cmos(void);
unsigned short get_cmos_memory_size(void);

#endif
