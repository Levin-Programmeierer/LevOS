#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PROCESSES 16

#define PROCESS_UNUSED  0
#define PROCESS_READY   1
#define PROCESS_RUNNING 2
#define PROCESS_DEAD    3

struct process {
    unsigned int pid;
    unsigned int state;

    unsigned int page_directory;

    unsigned int entry_point;
    unsigned int user_stack;

    unsigned int esp;
    unsigned int ebp;
};

void process_init(void);

int process_create(
    unsigned char *program,
    unsigned int size
);

void process_run(int pid);

void process_exit(void);

#endif