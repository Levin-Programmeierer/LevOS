#ifndef TERMINAL_H
#define TERMINAL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void terminal_init(void);
void print(const char *text, unsigned char color);
void putchar(char c);
void clear(void);
void backspace(void);
void print_hex_byte(unsigned char value);
void print_hex_dword(unsigned int value);

extern unsigned char WHITE;
extern unsigned char BLUE;
extern unsigned char GREEN;
extern unsigned char CYAN;
extern unsigned char RED;
extern unsigned char PURPLE;
extern unsigned char BROWN;
extern unsigned char GRAY;
extern unsigned char DARK_GRAY;
extern unsigned char LIGHT_BLUE;
extern unsigned char LIGHT_GREEN;
extern unsigned char LIGHT_CYAN;
extern unsigned char LIGHT_RED;
extern unsigned char LIGHT_PURPLE;
extern unsigned char YELLOW;

#endif