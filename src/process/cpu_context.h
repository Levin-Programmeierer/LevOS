#ifndef CPU_CONTEXT_H
#define CPU_CONTEXT_H

struct cpu_context {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int user_esp; /* ESP for user stack */
    unsigned int user_ss;  /* SS for user stack */

    unsigned int irq;
    unsigned int error;
};

#endif