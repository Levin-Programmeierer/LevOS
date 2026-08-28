#ifndef PROCESS_PROCESS_H
#define PROCESS_PROCESS_H

#include <stdint.h>
#include "process/cpu_context.h"

typedef void (*process_entry_t)(void *argument);

#define USER_CODE_ADDRESS  0x00400000u
#define USER_STACK_TOP     0x00800000u

enum process_state {
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_ZOMBIE
};

struct process {
    uint32_t pid;
    enum process_state state;
    uint32_t directory;
    uint32_t stack_base;
    struct cpu_context *context;
    process_entry_t entry;
    void *argument;
    uint8_t bootstrap;
    uint8_t user;
    uint32_t user_code_base;
    uint32_t user_stack_base;
};

void process_system_init(void);
int process_create(process_entry_t entry, void *argument);
int process_create_user(const uint8_t *image, uint32_t image_size);
void process_exit(void);
void process_terminate_current(void);
struct process *process_current(void);
struct process *process_by_pid(uint32_t pid);
uint32_t process_kernel_stack_top(const struct process *process);

#endif
