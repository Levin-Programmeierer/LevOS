#include "drivers/ata.h"
#include "drivers/io.h"

#define ATA_PRIMARY_IO       0x1F0
#define ATA_REG_DATA         0
#define ATA_REG_ERROR        1
#define ATA_REG_SECCOUNT0    2
#define ATA_REG_LBA0         3
#define ATA_REG_LBA1         4
#define ATA_REG_LBA2         5
#define ATA_REG_HDDEVSEL     6
#define ATA_REG_STATUS       7
#define ATA_REG_COMMAND      7
#define ATA_REG_ALTSTATUS    0x206

#define ATA_CMD_IDENTIFY     0xEC
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_SR_ERR            0x01
#define ATA_SR_DRQ            0x08
#define ATA_SR_DF             0x20
#define ATA_SR_BSY            0x80

#define ATA_PIO_EINVAL       -1
#define ATA_PIO_ENODEV       -2
#define ATA_PIO_ESTATUS      -3
#define ATA_PIO_ETIMEOUT     -4
#define ATA_PIO_EDRQ         -5

static uint8_t ata_status(ata_pio_device_t *device)
{
    return inb((unsigned short)(device->io_base + ATA_REG_STATUS));
}

static void ata_delay(ata_pio_device_t *device)
{
    /* Four alternate-status reads provide the required 400 ns delay. */
    inb((unsigned short)(device->io_base + ATA_REG_ALTSTATUS));
    inb((unsigned short)(device->io_base + ATA_REG_ALTSTATUS));
    inb((unsigned short)(device->io_base + ATA_REG_ALTSTATUS));
    inb((unsigned short)(device->io_base + ATA_REG_ALTSTATUS));
}

static int ata_fail(ata_pio_device_t *device, int error)
{
    device->last_error = error;
    if (error == ATA_PIO_ENODEV)
        device->present = 0;
    return error;
}

static int ata_wait(ata_pio_device_t *device, int data_request)
{
    uint8_t status;
    uint32_t timeout = 1000000;

    ata_delay(device);
    do {
        status = ata_status(device);
        /*
         * A floating IDE bus commonly reads as 0xFF, while QEMU reports
         * 0x00 for an unimplemented channel.  Neither is a usable ATA
         * device, and must not be mistaken for a timeout.
         */
        if (status == 0 || status == 0xFF)
            return ata_fail(device, ATA_PIO_ENODEV);
        if ((status & (ATA_SR_ERR | ATA_SR_DF)) != 0)
            return ata_fail(device, ATA_PIO_ESTATUS);
        if ((status & ATA_SR_BSY) == 0 &&
            (!data_request || (status & ATA_SR_DRQ) != 0))
            return 0;
    } while (--timeout != 0);
    return ata_fail(device, ATA_PIO_ETIMEOUT);
}

void ata_pio_init(ata_pio_device_t *device)
{
    uint8_t status;
    uint16_t identify[256];
    uint32_t sectors;
    uint32_t i;

    if (device == (ata_pio_device_t *)0)
        return;
    device->io_base = ATA_PRIMARY_IO;
    device->drive_select = 0xE0; /* LBA mode, primary master */
    device->present = 0;
    device->last_error = ATA_PIO_ENODEV;
    device->block.sector_size = 512;
    device->block.sector_count = 0;
    device->block.read_sector = ata_pio_read_sector;
    device->block.context = device;

    /*
     * Probe the selected drive with IDENTIFY.  This both distinguishes an
     * absent primary master from a disk that is still busy and gives the
     * block layer a useful capacity for bounds checking.
     */
    outb((unsigned short)(device->io_base + ATA_REG_HDDEVSEL),
         device->drive_select);
    ata_delay(device);
    status = ata_status(device);
    if (status == 0 || status == 0xFF) {
        device->last_error = ATA_PIO_ENODEV;
        return;
    }
    if (ata_wait(device, 0) != 0)
        return;
    outb((unsigned short)(device->io_base + ATA_REG_SECCOUNT0), 0);
    outb((unsigned short)(device->io_base + ATA_REG_LBA0), 0);
    outb((unsigned short)(device->io_base + ATA_REG_LBA1), 0);
    outb((unsigned short)(device->io_base + ATA_REG_LBA2), 0);
    outb((unsigned short)(device->io_base + ATA_REG_COMMAND), ATA_CMD_IDENTIFY);
    if (ata_wait(device, 1) != 0)
        return;

    for (i = 0; i < 256; ++i)
        identify[i] = inw((unsigned short)(device->io_base + ATA_REG_DATA));
    sectors = (uint32_t)identify[60] | ((uint32_t)identify[61] << 16);
    if (sectors != 0) {
        /* This driver deliberately issues only 28-bit LBA commands. */
        device->block.sector_count =
            sectors > 0x10000000u ? 0x10000000u : sectors;
    }
    device->present = 1;
    device->last_error = 0;
}

int ata_pio_is_present(const ata_pio_device_t *device)
{
    return device != (const ata_pio_device_t *)0 && device->present != 0;
}

int ata_pio_last_error(const ata_pio_device_t *device)
{
    return device == (const ata_pio_device_t *)0
               ? ATA_PIO_EINVAL : device->last_error;
}

const char *ata_pio_error_string(int error)
{
    switch (error) {
    case 0: return "ok";
    case ATA_PIO_EINVAL: return "invalid argument";
    case ATA_PIO_ENODEV: return "ATA primary-master device not present";
    case ATA_PIO_ESTATUS: return "ATA device reported an error";
    case ATA_PIO_ETIMEOUT: return "ATA device timed out";
    case ATA_PIO_EDRQ: return "ATA device did not request data";
    default: return "ATA I/O error";
    }
}

int ata_pio_read_sector(block_device_t *block, uint32_t sector,
                        uint8_t *buffer)
{
    ata_pio_device_t *device;
    uint8_t status;
    uint16_t *words;
    uint32_t i;

    if (block == (block_device_t *)0 || buffer == (uint8_t *)0 ||
        block->sector_size != 512 ||
        block->context == (void *)0 || sector > 0x0FFFFFFFu)
        return ATA_PIO_EINVAL;
    device = (ata_pio_device_t *)block->context;
    if (!device->present)
        return ata_fail(device, ATA_PIO_ENODEV);
    if (ata_wait(device, 0) != 0)
        return device->last_error;

    outb((unsigned short)(device->io_base + ATA_REG_HDDEVSEL),
         (unsigned char)(device->drive_select | ((sector >> 24) & 0x0F)));
    if (ata_wait(device, 0) != 0)
        return device->last_error;
    outb((unsigned short)(device->io_base + ATA_REG_SECCOUNT0), 1);
    outb((unsigned short)(device->io_base + ATA_REG_LBA0), (uint8_t)sector);
    outb((unsigned short)(device->io_base + ATA_REG_LBA1), (uint8_t)(sector >> 8));
    outb((unsigned short)(device->io_base + ATA_REG_LBA2), (uint8_t)(sector >> 16));
    outb((unsigned short)(device->io_base + ATA_REG_COMMAND), ATA_CMD_READ_SECTORS);

    if (ata_wait(device, 1) != 0)
        return device->last_error;
    status = inb((unsigned short)(device->io_base + ATA_REG_STATUS));
    if ((status & (ATA_SR_ERR | ATA_SR_DF)) != 0)
        return ata_fail(device, ATA_PIO_ESTATUS);
    if ((status & ATA_SR_DRQ) == 0)
        return ata_fail(device, ATA_PIO_EDRQ);

    words = (uint16_t *)(void *)buffer;
    for (i = 0; i < 256; ++i)
        words[i] = inw((unsigned short)(device->io_base + ATA_REG_DATA));
    device->last_error = 0;
    return 0;
}
