#include "drivers/keyboard.h"
#include "cpu/interrupts.h"
#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/cmos.h"
#include <stdint.h>

#define COMMAND_BUFFER_SIZE 128
#define COMMAND_SIZE 32
#define PARAMETER_SIZE 96

void process_command(void);
void split_command(const char *input, char *command, char *parameters);
int strcmp(const char *a, const char *b);

char command_buffer[COMMAND_BUFFER_SIZE];
char command[COMMAND_SIZE];
char argument[PARAMETER_SIZE];
unsigned int command_length = 0;

uint16_t background = 0x0;

void shell(void){
    clear(background);
    print("----------------------------------LevShell 0.1----------------------------------", (BLUE << 4) | WHITE); // 12 characters long
    print("\n> ", (background << 4) | WHITE);
    for (;;) {
        char c = keyboard_getchar();

        if (c == 0) {
            __asm__ volatile ("hlt");
            continue;
        }

        if (c == '\n') {
            command_buffer[command_length] = '\0';
            process_command();
            command_length = 0;
            print("\n> ", (background << 4) | WHITE);
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
        print("Available commands:", YELLOW);
        print("\nhelp -- this command", WHITE);
        print("\nclear -- clears the screen", WHITE);
        print("\nabout -- shows information about this OS", WHITE);
    }
    else if (strcmp(command_buffer, "clear") == 0) {
        clear(background);
        print("----------------------------------LevShell 0.1----------------------------------", (BLUE << 4) | WHITE);
    }
    else if(strcmp(command, "echo") == 0){
        print(argument, 0x0F);
    } else if(strcmp(command_buffer, "clock") == 0){
        //seconds
        select_cmos_reg(0x00);
        uint8_t seconds = read_cmos();
        //minutes
        select_cmos_reg(0x02);
        uint8_t minutes = read_cmos();
        print(minutes, YELLOW);
    }
    else if(strcmp(command_buffer, "about") == 0){
        print("--LevOS - Shell 0.1--", YELLOW);
        print("\nWork in progress\n", RED);
        print(".", (WHITE << 4));
        print(".", (RED << 4));
        print(".", (YELLOW << 4));
        print(".", (BLUE << 4));
        print(".", (GREEN << 4));
        print(".", (CYAN << 4));
        print(".", (BROWN << 4));
        print("\n.", (LIGHT_BLUE << 4));
        print(".", (LIGHT_CYAN << 4));
        print(".", (LIGHT_RED << 4));
        print(".", (LIGHT_GREEN << 4));
        print(".", (GRAY << 4));
        print(".", (DARK_GRAY << 4));
        print(".", (LIGHT_PURPLE << 4));
    }
    
    else {
        print("Unknown command: ", 0xAA);
        print(command, 0xAA);
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

    return (a[i] != b[i]);
}

void split_command(const char *input, char *command, char *parameters)
{
    int command_index = 0;
    int parameter_index = 0;
    int command_done = 0;

    for (unsigned int i = 0; i < command_length; i++) {

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