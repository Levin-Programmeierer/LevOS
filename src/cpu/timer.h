#ifndef TIMER_H
#define TIMER_H

void timer_init(unsigned int frequency);
struct cpu_context *timer_handler(struct cpu_context *context);
unsigned int timer_ticks(void);

#endif