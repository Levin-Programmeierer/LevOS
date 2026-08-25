#ifndef TIMER_H
#define TIMER_H

void timer_init(unsigned int frequency);
void timer_handler(void);
unsigned int timer_ticks(void);

#endif