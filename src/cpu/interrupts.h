#ifndef IO_H
#define IO_H

#include "process/cpu_context.h"

void initialise_GDT(void);
void exception_handler(unsigned int exception, unsigned int error_code);
void initialise_IDT(void);
void cpu_halt(void);
struct cpu_context *irq_handler(
    struct cpu_context *context
);

#endif