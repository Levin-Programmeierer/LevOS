#ifndef CPU_TIMER_H
#define CPU_TIMER_H

#include <stdint.h>
#include "process/cpu_context.h"

void timer_init(uint32_t frequency);
struct cpu_context *timer_handler(struct cpu_context *context);
uint32_t timer_ticks(void);

#endif
