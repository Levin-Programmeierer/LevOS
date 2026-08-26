#include "drivers/framebuffer_driver.h"
#include <stdint.h>
#include "drivers/font.h"

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;

    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;

    uint8_t red_position;
    uint8_t red_mask_size;

    uint8_t green_position;
    uint8_t green_mask_size;

    uint8_t blue_position;
    uint8_t blue_mask_size;
};

static uint8_t *framebuffer = 0;

static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint8_t fb_bpp = 0;

static uint8_t red_position = 0;
static uint8_t green_position = 0;
static uint8_t blue_position = 0;


int framebuffer_init(uint32_t mbi_addr)
{
    struct multiboot_tag *tag =
        (struct multiboot_tag *)(mbi_addr + 8);

    while (tag->type != 0) {

        if (tag->type == 8) {

            struct multiboot_tag_framebuffer *fb =
                (struct multiboot_tag_framebuffer *)tag;

            print("FRAMEBUFFER TAG FOUND\n", 0x0F);

            /*
             * We want a direct RGB framebuffer.
             */
            print("Framebuffer type: ", 0x0F);
            print_hex_dword(fb->framebuffer_type);
            print("\n", 0x0F);

            /*
             * This is a 32-bit kernel.
             */
            if (fb->framebuffer_addr > 0xFFFFFFFFULL) {
                print("Framebuffer address too high\n", 0x0F);
                return 0;
            }

            /*
             * Save framebuffer information.
             */
            framebuffer =
                (uint8_t *)(uint32_t)fb->framebuffer_addr;

            fb_width =
                fb->framebuffer_width;

            fb_height =
                fb->framebuffer_height;

            fb_pitch =
                fb->framebuffer_pitch;

            fb_bpp =
                fb->framebuffer_bpp;

            red_position =
                fb->red_position;

            green_position =
                fb->green_position;

            blue_position =
                fb->blue_position;


            /*
             * Print the information.
             */
            print("Framebuffer address: ", 0x0F);
            print_hex_dword(
                (uint32_t)fb->framebuffer_addr
            );
            print("\n", 0x0F);

            print("Width: ", 0x0F);
            print_hex_dword(fb->framebuffer_width);
            print("\n", 0x0F);

            print("Height: ", 0x0F);
            print_hex_dword(fb->framebuffer_height);
            print("\n", 0x0F);

            print("Pitch: ", 0x0F);
            print_hex_dword(fb->framebuffer_pitch);
            print("\n", 0x0F);

            print("BPP: ", 0x0F);
            print_hex_dword(fb->framebuffer_bpp);
            print("\n", 0x0F);

            print("Red position: ", 0x0F);
            print_hex_dword(fb->red_position);
            print("\n", 0x0F);

            print("Green position: ", 0x0F);
            print_hex_dword(fb->green_position);
            print("\n", 0x0F);

            print("Blue position: ", 0x0F);
            print_hex_dword(fb->blue_position);
            print("\n", 0x0F);

            return 1;
        }

        /*
         * Multiboot2 tags are aligned to 8 bytes.
         */
        tag = (struct multiboot_tag *)
            ((uint8_t *)tag +
             ((tag->size + 7) & ~7));
    }

    print("NO FRAMEBUFFER\n", 0x0F);

    return 0;
}


void putpixel(int x, int y, uint32_t color)
{
    if (framebuffer == 0)
        return;

    if (x < 0 || y < 0)
        return;

    if ((uint32_t)x >= fb_width ||
        (uint32_t)y >= fb_height)
        return;

    if (fb_bpp != 32)
        return;

    uint32_t red =
        (color >> 16) & 0xFF;

    uint32_t green =
        (color >> 8) & 0xFF;

    uint32_t blue =
        color & 0xFF;

    uint32_t pixel =
        (red << red_position) |
        (green << green_position) |
        (blue << blue_position);

    uint32_t *location =
        (uint32_t *)(framebuffer +
                     y * fb_pitch +
                     x * 4);

    *location = pixel;
}


void fill_rect(
    int x,
    int y,
    int width,
    int height,
    uint32_t color)
{
    for (int py = y; py < y + height; py++) {
        for (int px = x; px < x + width; px++) {
            putpixel(px, py, color);
        }
    }
}

void draw_char(
    int x,
    int y,
    unsigned char c,
    uint32_t color)
{
    const uint8_t *glyph = font_get_glyph(c);

    if (!glyph)
        return;

    for (int row = 0; row < 16; row++) {

        uint8_t bits = glyph[row];

        for (int col = 0; col < 8; col++) {

            if (bits & (0x80 >> col)) {
                putpixel(
                    x + col,
                    y + row,
                    color
                );
            }
        }
    }
}

void draw_string(
    int x,
    int y,
    const char *str,
    uint32_t color)
{
    int start_x = x;

    while (*str) {

        if (*str == '\n') {
            x = start_x;
            y += 16;
        }
        else if (*str == '\r') {
            x = start_x;
        }
        else if (*str == '\t') {
            x += 32;
        }
        else {
            draw_char(
                x,
                y,
                (unsigned char)*str,
                color
            );

            x += 8;
        }

        str++;
    }
}