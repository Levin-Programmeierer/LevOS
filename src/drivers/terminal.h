#ifndef TERMINAL_H
#define TERMINAL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include <stdint.h>

void terminal_init(void);
void print(const char *text, uint16_t color);
void putchar(char c);
void clear(uint16_t color);
void backspace(void);
void print_hex_byte(unsigned char value);
void print_hex_dword(unsigned int value);
void print_number(unsigned int value);

extern uint16_t WHITE;
extern uint16_t BLUE;
extern uint16_t GREEN;
extern uint16_t CYAN;
extern uint16_t RED;
extern uint16_t PURPLE;
extern uint16_t BROWN;
extern uint16_t GRAY;
extern uint16_t DARK_GRAY;
extern uint16_t LIGHT_BLUE;
extern uint16_t LIGHT_GREEN;
extern uint16_t LIGHT_CYAN;
extern uint16_t LIGHT_RED;
extern uint16_t LIGHT_PURPLE;
extern uint16_t YELLOW;
extern uint16_t BLACK;

#endif