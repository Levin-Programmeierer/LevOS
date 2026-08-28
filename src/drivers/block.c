#include "drivers/block.h"

int block_read_sector(block_device_t *device, uint32_t sector,
                      uint8_t *buffer)
{
    if (device == (block_device_t *)0 || device->read_sector == (block_read_sector_fn)0 ||
        buffer == (uint8_t *)0 || device->sector_size == 0)
        return -1;
    if (device->sector_count != 0 && sector >= device->sector_count)
        return -1;
    return device->read_sector(device, sector, buffer);
}
