#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "drivers/block.h"

#define FAT32_MAX_SECTOR_SIZE 4096
#define FAT32_NAME_SIZE 13

typedef enum {
    FAT32_OK = 0,
    FAT32_EINVAL = -1,
    FAT32_EIO = -2,
    FAT32_EBADFS = -3,
    FAT32_ENOTFOUND = -4,
    FAT32_ENOTDIR = -5,
    FAT32_EISDIR = -6,
    FAT32_EOVERFLOW = -7,
    FAT32_ELOOP = -8,
    FAT32_ENOSPC = -9
} fat32_result_t;

typedef struct {
    char name[FAT32_NAME_SIZE];
    uint8_t attributes;
    uint32_t size;
    uint32_t first_cluster;
} fat32_dirent_t;

typedef int (*fat32_dir_callback_t)(const fat32_dirent_t *entry, void *context);

typedef struct {
    block_device_t *device;
    uint32_t volume_start;
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t fat_start;
    uint32_t fat_sectors;
    uint32_t data_start;
    uint32_t cluster_count;
    uint32_t root_cluster;
    uint8_t sector_buffer[FAT32_MAX_SECTOR_SIZE];
    int mounted;
} fat32_fs_t;

fat32_result_t fat32_mount(fat32_fs_t *fs, block_device_t *device);
fat32_result_t fat32_list_directory(fat32_fs_t *fs, const char *path,
                                     fat32_dir_callback_t callback,
                                     void *context);
fat32_result_t fat32_read_file(fat32_fs_t *fs, const char *path,
                               uint8_t *buffer, uint32_t capacity,
                               uint32_t *bytes_read);
const char *fat32_result_string(fat32_result_t result);

#endif
