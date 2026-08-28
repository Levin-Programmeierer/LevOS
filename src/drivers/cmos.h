#ifndef DRIVERS_CMOS_H
#define DRIVERS_CMOS_H

void select_cmos_reg(unsigned char reg);
unsigned char read_cmos(void);

#endif
