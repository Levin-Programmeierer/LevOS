#include "drivers/keyboard.h"
#include "drivers/font.h"
#include "cpu/interrupts.h"
#include "drivers/framebuffer_driver.h"
#define COMMAND_BUFFER_SIZE 128
#define COMMAND_SIZE 32
#define PARAMETER_SIZE 96

char command_buffer[COMMAND_BUFFER_SIZE];
char command[COMMAND_SIZE];
char argument[PARAMETER_SIZE];
unsigned int command_length = 0;

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

#define FONT_WIDTH  8
#define FONT_HEIGHT 16

unsigned int BG_COLOR = 0x000000;
unsigned int TEXT_COLOR = 0xFFFFFF;
#define PROMPT_COLOR 0x00FF00

unsigned int cursorx = 0;
unsigned int cursory = FONT_HEIGHT * 3;

unsigned int red = 0xFF0000;
unsigned int yellow = 0xFFFF00;
unsigned int white = 0xFFFFFF;
unsigned int green = 0x00FF00;
unsigned int blue = 0x0000FF;
unsigned int violet = 0x00FFFF;
unsigned int pink = 0xFF00FF;
unsigned int orange = 0xFFBB00;

unsigned int BACKCOLOR = 0x000000;

void graphicsshell(void){
    graphics_clear();
    graphics_prompt();

    for (;;) {
        fill_rect(0, 0, 1024, 26, 0x0000AA);
        draw_string(SCREEN_WIDTH / 2 - 30 * 9 / 2, 5, "LevOS - V0.2 - Graphical shell", 0xFFFF00);
        char c = keyboard_getchar();

        if (c == 0) {
            cpu_halt();
            continue;
        }

        if (c == '\n') {
            command_buffer[command_length] = '\0';
            process_command();
            command_length = 0;
            graphics_prompt();
        }
        else if (c == '\b') {
            if (command_length > 0) {
                command_length--;
                command_buffer[command_length] = '\0';
                fill_rect(cursorx -= FONT_WIDTH, cursory, FONT_WIDTH, FONT_HEIGHT, BG_COLOR);
            }
        }
        else if (command_length < COMMAND_BUFFER_SIZE - 1) {
            command_buffer[command_length] = c;
            command_length++;
            previewchar(c);
        }
    }
}

void graphics_clear(void){
    fill_rect(
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        BG_COLOR
    );

    cursorx = 0;
    cursory = FONT_HEIGHT * 3;
}

void previewchar(unsigned char c){
    if (cursorx + FONT_WIDTH > 1024){
        cursorx = 0;
        if(cursory + FONT_HEIGHT < 768)
            cursory += FONT_HEIGHT + 5;
        else{
            graphics_clear();
            cursory = FONT_HEIGHT * 3;
        }
    }
    draw_char(cursorx, cursory, c, TEXT_COLOR);
    cursorx += FONT_WIDTH;
}

void graphics_print(const char *str, uint32_t color){
    draw_string(
        cursorx,
        cursory,
        str,
        color
    );
}

void graphics_prompt(void) {
    cursorx = 0;
    cursory += FONT_HEIGHT + 5;
    graphics_print("LevOS > ", PROMPT_COLOR);
    cursorx = 8*9;
}

void process_command(void){
    split_command(command_buffer, command, argument);
    putchar('\n');
    if (strcmp(command_buffer, "help") == 0) {

    }
    else if (strcmp(command_buffer, "clear") == 0) {
        graphics_clear();
    }
    else if(strcmp(command, "echo") == 0){

    } 
    else if(strcmp(command_buffer, "about") == 0){
        
        fill_rect(300, 200, 50, 50, 0xFF0000);
        fill_rect(350, 200, 50, 50, 0xFFFF00);
        fill_rect(400, 200, 50, 50, 0xFFFFFF);
        fill_rect(450, 200, 50, 50, 0x00FF00);
        fill_rect(500, 200, 50, 50, 0x0000FF);
        fill_rect(550, 200, 50, 50, 0x00FFFF);
        fill_rect(600, 200, 50, 50, 0xFF00FF);
        fill_rect(650, 200, 50, 50, 0xFFBB00);
    }
    else if(strcmp(command, "background") == 0){
        if (strcmp(argument, "red") == 0){
            BG_COLOR = red;
        } else if (strcmp(argument, "blue") == 0){
            BG_COLOR = blue;
        } else if (strcmp(argument, "green") == 0){
            BG_COLOR = green;
        } else if (strcmp(argument, "yellow") == 0){
            BG_COLOR = yellow;
            TEXT_COLOR = 0x000000;
        } else if (strcmp(argument, "white") == 0){
            BG_COLOR = white;
            TEXT_COLOR = 0x000000;
        } else if (strcmp(argument, "orange") == 0){
            BG_COLOR = orange;
            TEXT_COLOR = 0x000000;
        }
        
        graphics_clear();
    }
    
    else {
        fill_rect(0, 0, 30, 30, 0x00FF00);
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