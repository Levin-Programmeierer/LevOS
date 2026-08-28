#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

/*
 * A block device exposes logical sectors.  The callback returns zero on
 * success and a negative value on I/O failure.  The buffer is sector_size
 * bytes long and must be writable by the caller.
 */
typedef struct block_device block_device_t;
typedef int (*block_read_sector_fn)(block_device_t *device,
                                    uint32_t sector, uint8_t *buffer);

struct block_device {
    uint32_t sector_size;
    uint32_t sector_count; /* zero means that the size is not known */
    block_read_sector_fn read_sector;
    void *context;
};

int block_read_sector(block_device_t *device, uint32_t sector,
                      uint8_t *buffer);

#endif
