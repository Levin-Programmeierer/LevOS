#ifndef MEMORY_GDT_H
#define MEMORY_GDT_H

#include <stdint.h>

void init_GDT(void);
void tss_set_kernel_stack(uint32_t stack_top);

#endif
