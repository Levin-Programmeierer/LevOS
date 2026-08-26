#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process/cpu_context.h"

void scheduler_init(void);

struct cpu_context *scheduler_tick(
    struct cpu_context *context
);

/* Scratch page used for safe context switching: virtual address visible
   under each process' page directory. */
extern unsigned int scheduler_scratch_phys;
extern unsigned int scheduler_scratch_vaddr;

/* Directory that the IRQ assembly should load into CR3 when performing the
   context switch. */
extern unsigned int scheduler_next_directory;

#endif