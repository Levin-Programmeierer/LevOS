#ifndef PROCESS_H
#define PROCESS_H

#include "process/cpu_context.h"

#define MAX_PROCESSES 16

#define PROCESS_UNUSED   0
#define PROCESS_READY    1
#define PROCESS_RUNNING  2
#define PROCESS_BLOCKED  3
#define PROCESS_DEAD     4

struct process {
    unsigned int pid;
    unsigned int state;

    unsigned int page_directory;

    unsigned int entry_point;
    unsigned int user_stack;

    unsigned int esp;
    unsigned int ebp;

    unsigned int kernel_stack;

    struct cpu_context context;
};

void process_init(void);

int process_create(
    unsigned char *program,
    unsigned int size
);

void process_run(int pid);

void process_exit(void);

void process_save_context(
    int pid,
    struct cpu_context *context
);

struct cpu_context *process_get_context(
    int pid
);

unsigned int process_get_page_directory(int pid);
int process_next_ready(int current_pid);

#endif