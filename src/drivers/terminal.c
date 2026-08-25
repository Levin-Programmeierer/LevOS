#include "drivers/terminal.h"
#include "drivers/io.h"

int cursor_x = 0;
int cursor_y = 0;

unsigned char WHITE = 0x0F;
unsigned char BLUE = 0x01;
unsigned char GREEN = 0x02;
unsigned char CYAN = 0x03;
unsigned char RED = 0x04;
unsigned char PURPLE = 0x0A;
unsigned char BROWN = 0x06;
unsigned char GRAY = 0x07;
unsigned char DARK_GRAY = 0x08;
unsigned char LIGHT_BLUE = 0x09;
unsigned char LIGHT_GREEN = 0x02;
unsigned char LIGHT_CYAN = 0x0B;
unsigned char LIGHT_RED = 0x0C;
unsigned char LIGHT_PURPLE = 0x05;
unsigned char YELLOW = 0x0E;

volatile unsigned short *video =
    (unsigned short *)0xB8000;

static void scroll(void) {
    if (cursor_y < VGA_HEIGHT) {
        return;
    }

    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            video[(y - 1) * VGA_WIDTH + x] =
                video[y * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        video[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            0x0F20;
    }

    cursor_y = VGA_HEIGHT - 1;
}

void terminal_init(void) {
    cursor_x = 0;
    cursor_y = 0;

    clear();
}

void print(const char *text, unsigned char color) {
    for (int i = 0; i < 100 && text[i] != '\0'; i++) {

        if (text[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        }
        else {
            int position =
                cursor_y * VGA_WIDTH + cursor_x;

            video[position] =
                (color << 8) | text[i];

            cursor_x++;

            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
        }

        scroll();
    }
}

void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    }
    else if (c == '\b') {
        backspace();
        return;
    }
    else {
        int position =
            cursor_y * VGA_WIDTH + cursor_x;

        video[position] =
            (0x0F << 8) | c;

        cursor_x++;

        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    scroll();
}

void clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video[i] = 0x0F20;
    }

    cursor_x = 0;
    cursor_y = 0;
}

void backspace(void) {
    if (cursor_x > 0) {
        cursor_x--;

        video[cursor_y * VGA_WIDTH + cursor_x] =
            0x0F20;
    }
}

void print_hex_byte(unsigned char value) {
    const char hex[] =
        "0123456789ABCDEF";

    putchar(hex[(value >> 4) & 0xF]);
    putchar(hex[value & 0xF]);
}

void print_hex_dword(unsigned int value) {
    const char hex[] =
        "0123456789ABCDEF";

    putchar('0');
    putchar('x');

    for (int i = 28; i >= 0; i -= 4) {
        putchar(hex[(value >> i) & 0xF]);
    }
}