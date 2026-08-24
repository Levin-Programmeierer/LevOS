#ifndef IO_H
#define IO_H

void initialise_GDT(void);
void exception_handler(unsigned int exception, unsigned int error_code);
void initialise_IDT(void);
void cpu_halt(void);

#endif