#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "process/cpu_context.h"

void init_IDT(void);
struct cpu_context *isr_common_handler(struct cpu_context *context);
struct cpu_context *syscall_dispatch(struct cpu_context *context);

#endif