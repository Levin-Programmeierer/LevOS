#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include "drivers/block.h"

typedef struct {
    block_device_t block;
    uint16_t io_base;
    uint8_t drive_select;
    uint8_t present;
    int last_error;
} ata_pio_device_t;

/* Configure the primary-master ATA device (28-bit LBA, 512-byte sectors). */
void ata_pio_init(ata_pio_device_t *device);
int ata_pio_is_present(const ata_pio_device_t *device);
int ata_pio_last_error(const ata_pio_device_t *device);
const char *ata_pio_error_string(int error);
int ata_pio_read_sector(block_device_t *block, uint32_t sector,
                        uint8_t *buffer);

#endif
