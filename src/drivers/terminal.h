#ifndef TERMINAL_H
#define TERMINAL_H

void print(const char *text);
void clear(void);
void putchar(char c);
void print_hex_byte(unsigned char value);
void print_hex_dword(unsigned int error_code);

#endif