#ifndef FRAMEBUFFER_DRIVER_H
#define FRAMEBUFFER_DRIVER_H

#include <stdint.h>

int framebuffer_init(uint32_t mbi_addr);

void putpixel(int x, int y, uint32_t color);

void fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color
);

void draw_string(
    int x,
    int y,
    const char *str,
    uint32_t color);

void draw_char(
int x,
int y,
unsigned char c,
uint32_t color);

#endif