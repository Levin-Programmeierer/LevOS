#include "process/process.h"
#include <stdint.h>

static const uint8_t user_test_image[] = {
    0xb8, 0x01, 0x00, 0x00, 0x00,             /* write */
    0xbb, 0x01, 0x00, 0x00, 0x00,             /* stdout */
    0xb9, 0x20, 0x00, 0x40, 0x00,             /* message address */
    0xba, 0x13, 0x00, 0x00, 0x00,             /* message length */
    0xcd, 0x80,
    0xb8, 0x03, 0x00, 0x00, 0x00,             /* yield */
    0xcd, 0x80,
    0xeb, 0xf7,                                /* yield forever */
    0x90,                                      /* message offset 32 */
    'H', 'e', 'l', 'l', 'o', ' ', 'f', 'r', 'o', 'm', ' ',
    'r', 'i', 'n', 'g', ' ', '3', '!', '\n'
};

int user_test_create(void)
{
    return process_create_user(user_test_image, sizeof(user_test_image));
}
