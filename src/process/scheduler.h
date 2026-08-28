#ifndef PROCESS_SCHEDULER_H
#define PROCESS_SCHEDULER_H

#include <stdint.h>
#include "process/cpu_context.h"

void scheduler_init(void);
struct cpu_context *scheduler_tick(struct cpu_context *context);
struct cpu_context *scheduler_yield(struct cpu_context *context);

extern volatile uint32_t scheduler_next_directory;

#endif
