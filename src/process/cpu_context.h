#ifndef PROCESS_CPU_CONTEXT_H
#define PROCESS_CPU_CONTEXT_H

#include <stdint.h>

/*
 * This is the stack image made by idt.asm.  Keep the order in sync with
 * pusha and with the three values consumed by iretd.
 */
struct cpu_context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t vector;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t user_ss;
};

#endif
