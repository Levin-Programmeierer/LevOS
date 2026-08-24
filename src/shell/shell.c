#include "drivers/keyboard.h"
#include "drivers/terminal.h"
#include "cpu/interrupts.h"
#define COMMAND_BUFFER_SIZE 128
#define COMMAND_SIZE 32
#define PARAMETER_SIZE 96

char command_buffer[COMMAND_BUFFER_SIZE];
char command[COMMAND_SIZE];
char argument[PARAMETER_SIZE];
unsigned int command_length = 0;

void shell(void){
    for (;;) {
        char c = keyboard_getchar();

        if (c == 0) {
            cpu_halt();
            continue;
        }

        if (c == '\n') {
            command_buffer[command_length] = '\0';
            process_command();
            command_length = 0;
            print("\n> ");
        }
        else if (c == '\b') {
            if (command_length > 0) {
            command_length--;
            command_buffer[command_length] = '\0';
            putchar('\b');
            }
        }
        else if (command_length < COMMAND_BUFFER_SIZE - 1) {
            command_buffer[command_length] = c;
            command_length++;
            putchar(c);
        }
    }
}

void process_command(void){
    split_command(command_buffer, command, argument);
    putchar('\n');
    if (strcmp(command_buffer, "help") == 0) {
        print("Available commands:");
        print("\nhelp -- this command");
        print("\nclear -- clears the screen");
    }
    else if (strcmp(command_buffer, "clear") == 0) {
        clear();
    }
    else if(strcmp(command, "echo") == 0){
        print(argument);
    } 
    else if(strcmp(command_buffer, "about") == 0){
        print("--LevOS - Shell 0.1--");
        print("\nWork in progress");
    }
    
    else {
        print("Unknown command: ");
        print(command);
    }
}

int strcmp(const char *a, const char *b){
    int i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return 1;
        }

        i++;
    }

    return a[i] != b[i];
}

void split_command(const char *input, char *command, char *parameters)
{
    int command_index = 0;
    int parameter_index = 0;
    int command_done = 0;

    for (int i = 0; i < command_length; i++) {

        if (input[i] != ' ' && command_done == 0) {

            command[command_index] = input[i];
            command_index++;

        } else if (command_done == 0) {

            command[command_index] = '\0';
            command_done = 1;

        } else {

            parameters[parameter_index] = input[i];
            parameter_index++;
        }
    }

    command[command_index] = '\0';
    parameters[parameter_index] = '\0';
}