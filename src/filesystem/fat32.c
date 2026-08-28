#include "filesystem/fat32.h"

static uint16_t fat16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t fat32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int power_of_two(uint32_t value)
{
    return value != 0 && (value & (value - 1)) == 0;
}

/*
 * The FAT type is identified by the BPB layout, not just by the number of
 * clusters.  mkfs.fat -F32 is allowed to create a small FAT32 volume (with a
 * cluster count below the usual FAT32 threshold), which is still unambiguous
 * because the FAT16 BPB fields are zero and the FAT32 fields are populated.
 */
static int looks_like_fat32_bpb(const uint8_t *bpb, uint32_t sector_size)
{
    uint32_t bytes_per_sector;

    if (bpb == (const uint8_t *)0 ||
        fat16(bpb + 510) != 0xAA55)
        return 0;
    bytes_per_sector = fat16(bpb + 11);
    if (bytes_per_sector != sector_size ||
        bytes_per_sector < 512 ||
        bytes_per_sector > FAT32_MAX_SECTOR_SIZE ||
        !power_of_two(bytes_per_sector) ||
        !power_of_two(bpb[13]) ||
        bpb[13] > 128 ||
        fat16(bpb + 14) == 0 ||
        bpb[16] == 0 || bpb[16] > 2 ||
        fat16(bpb + 17) != 0 ||
        fat16(bpb + 19) != 0 ||
        fat32(bpb + 32) == 0 ||
        fat32(bpb + 36) == 0)
        return 0;
    return 1;
}

static fat32_result_t read_sector(fat32_fs_t *fs, uint32_t sector)
{
    uint32_t physical_sector;
    if (!fs->mounted ||
        (fs->device->sector_count != 0 &&
         (fs->volume_start > fs->device->sector_count ||
          sector >= fs->device->sector_count - fs->volume_start)) ||
        sector > 0xFFFFFFFFu - fs->volume_start)
        return FAT32_EIO;
    physical_sector = fs->volume_start + sector;
    return block_read_sector(fs->device, physical_sector, fs->sector_buffer) == 0
               ? FAT32_OK : FAT32_EIO;
}

static int valid_cluster(const fat32_fs_t *fs, uint32_t cluster)
{
    return cluster >= 2 && cluster - 2 < fs->cluster_count;
}

static fat32_result_t next_cluster(fat32_fs_t *fs, uint32_t cluster,
                                   uint32_t *next)
{
    uint32_t offset;
    uint32_t sector;
    uint32_t in_sector;
    uint32_t value;

    if (!valid_cluster(fs, cluster) || next == (uint32_t *)0)
        return FAT32_EBADFS;
    offset = cluster * 4u;
    if (offset / fs->bytes_per_sector >= fs->fat_sectors)
        return FAT32_EBADFS;
    sector = fs->fat_start + offset / fs->bytes_per_sector;
    in_sector = offset % fs->bytes_per_sector;
    if (in_sector + 4 > fs->bytes_per_sector)
        return FAT32_EBADFS;
    if (read_sector(fs, sector) != FAT32_OK)
        return FAT32_EIO;
    value = fat32(fs->sector_buffer + in_sector) & 0x0FFFFFFFu;
    if (value >= 0x0FFFFFF8u) {
        *next = 0x0FFFFFFFu;
        return FAT32_OK;
    }
    if (value == 0x0FFFFFF7u || (value >= 0x0FFFFFF0u && value < 0x0FFFFFF8u))
        return FAT32_EBADFS;
    if (!valid_cluster(fs, value))
        return FAT32_EBADFS;
    *next = value;
    return FAT32_OK;
}

static void uppercase(char *text)
{
    uint32_t i;
    for (i = 0; text[i] != '\0'; ++i)
        if (text[i] >= 'a' && text[i] <= 'z')
            text[i] = (char)(text[i] - 'a' + 'A');
}

static int component(const char *path, uint32_t *position, char *out)
{
    uint32_t i = 0;
    uint32_t p;

    if (path == (const char *)0 || position == (uint32_t *)0 ||
        out == (char *)0)
        return -1;
    p = *position;
    while (path[p] == '/' || path[p] == '\\')
        ++p;
    if (path[p] == '\0') {
        *position = p;
        out[0] = '\0';
        return 0;
    }
    while (path[p] != '\0' && path[p] != '/' && path[p] != '\\') {
        if (i >= FAT32_NAME_SIZE - 1)
            return -1;
        out[i++] = path[p++];
    }
    out[i] = '\0';
    uppercase(out);
    *position = p;
    return 1;
}

static int names_equal(const char *a, const char *b)
{
    uint32_t i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char left = a[i];
        char right = b[i];
        if (left >= 'a' && left <= 'z')
            left = (char)(left - 'a' + 'A');
        if (right >= 'a' && right <= 'z')
            right = (char)(right - 'a' + 'A');
        if (left != right)
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

static int entry_from_raw(const uint8_t *raw, fat32_dirent_t *entry)
{
    uint32_t i;
    uint32_t base_end = 8;
    uint32_t ext_end = 11;
    uint32_t out = 0;

    if (raw[0] == 0x00 || raw[0] == 0xE5 || raw[11] == 0x0F ||
        (raw[11] & 0x08) != 0)
        return 0;
    while (base_end > 0 && raw[base_end - 1] == ' ')
        --base_end;
    while (ext_end > 8 && raw[ext_end - 1] == ' ')
        --ext_end;
    for (i = 0; i < base_end; ++i)
        entry->name[out++] = (char)raw[i];
    if (ext_end > 8) {
        entry->name[out++] = '.';
        for (i = 8; i < ext_end; ++i)
            entry->name[out++] = (char)raw[i];
    }
    entry->name[out] = '\0';
    entry->attributes = raw[11];
    entry->size = fat32(raw + 28);
    entry->first_cluster = ((uint32_t)fat16(raw + 20) << 16) | fat16(raw + 26);
    return 1;
}

typedef struct {
    const char *target;
    fat32_dirent_t *entry;
    int found;
} find_context_t;

static int find_callback(const fat32_dirent_t *entry, void *context)
{
    find_context_t *find = (find_context_t *)context;
    if (names_equal(entry->name, find->target)) {
        *find->entry = *entry;
        find->found = 1;
        return 1;
    }
    return 0;
}

static fat32_result_t scan_directory(fat32_fs_t *fs, uint32_t first_cluster,
                                      fat32_dir_callback_t callback,
                                      void *context)
{
    uint32_t cluster = first_cluster;
    uint32_t visited = 0;
    uint32_t sector_in_cluster;
    uint32_t entry_offset;

    if (!valid_cluster(fs, cluster))
        return FAT32_EBADFS;
    while (valid_cluster(fs, cluster)) {
        if (++visited > fs->cluster_count)
            return FAT32_ELOOP;
        for (sector_in_cluster = 0;
             sector_in_cluster < fs->sectors_per_cluster; ++sector_in_cluster) {
            uint32_t sector = fs->data_start +
                              (cluster - 2) * fs->sectors_per_cluster +
                              sector_in_cluster;
            if (read_sector(fs, sector) != FAT32_OK)
                return FAT32_EIO;
            for (entry_offset = 0; entry_offset + 32 <= fs->bytes_per_sector;
                 entry_offset += 32) {
                uint8_t *raw = fs->sector_buffer + entry_offset;
                fat32_dirent_t entry;
                if (raw[0] == 0x00)
                    return FAT32_OK;
                if (entry_from_raw(raw, &entry) &&
                    callback(&entry, context) != 0)
                    return FAT32_OK;
            }
        }
        if (next_cluster(fs, cluster, &cluster) != FAT32_OK)
            return FAT32_EBADFS;
        if (cluster == 0x0FFFFFFFu)
            return FAT32_OK;
    }
    return FAT32_EBADFS;
}

static fat32_result_t lookup(fat32_fs_t *fs, const char *path,
                             fat32_dirent_t *entry)
{
    uint32_t position = 0;
    uint32_t directory = fs->root_cluster;
    char part[FAT32_NAME_SIZE];
    int has_component;

    if (path == (const char *)0 || entry == (fat32_dirent_t *)0)
        return FAT32_EINVAL;
    for (;;) {
        has_component = component(path, &position, part);
        if (has_component < 0)
            return FAT32_EINVAL;
        if (has_component == 0) {
            if (position == 0 || directory == fs->root_cluster) {
                entry->name[0] = '\0';
                entry->attributes = 0x10;
                entry->size = 0;
                entry->first_cluster = directory;
                return FAT32_OK;
            }
            return FAT32_ENOTFOUND;
        }
        if (names_equal(part, "."))
            continue;
        {
            find_context_t find = { part, entry, 0 };
            fat32_result_t result = scan_directory(fs, directory,
                                                    find_callback, &find);
            if (result != FAT32_OK)
                return result;
            if (!find.found)
                return FAT32_ENOTFOUND;
        }
        while (path[position] == '/' || path[position] == '\\')
            ++position;
        if (path[position] != '\0') {
            if ((entry->attributes & 0x10) == 0)
                return FAT32_ENOTDIR;
            if (!valid_cluster(fs, entry->first_cluster))
                return FAT32_EBADFS;
            directory = entry->first_cluster;
        } else {
            return FAT32_OK;
        }
    }
}

fat32_result_t fat32_mount(fat32_fs_t *fs, block_device_t *device)
{
    uint32_t volume_start = 0;
    uint32_t volume_sectors = 0;
    uint32_t total_sectors;
    uint32_t fat_size;
    uint32_t data_sectors;
    uint32_t clusters;
    uint64_t fat_entries;
    uint64_t data_start;
    int found_volume = 0;

    if (fs == (fat32_fs_t *)0 || device == (block_device_t *)0 ||
        device->read_sector == (block_read_sector_fn)0 ||
        device->sector_size < 512 ||
        device->sector_size > FAT32_MAX_SECTOR_SIZE)
        return FAT32_EINVAL;
    fs->mounted = 0;
    fs->device = device;
    if (block_read_sector(device, 0, fs->sector_buffer) != 0)
        return FAT32_EIO;
    /*
     * Accept both a super-floppy volume and a conventional MBR partition.
     * Validate the complete FAT32 BPB before treating sector zero as a
     * volume; otherwise bytes in an MBR can be mistaken for BPB fields.
     */
    if (looks_like_fat32_bpb(fs->sector_buffer, device->sector_size)) {
        found_volume = 1;
    } else {
        uint32_t partition_start[4];
        uint32_t partition_size[4];
        uint8_t partition_type[4];

        if (fat16(fs->sector_buffer + 510) != 0xAA55)
            return FAT32_EBADFS;
        for (uint32_t partition = 0; partition < 4; ++partition) {
            const uint8_t *part = fs->sector_buffer + 446 + partition * 16;
            partition_type[partition] = part[4];
            partition_start[partition] = fat32(part + 8);
            partition_size[partition] = fat32(part + 12);
        }
        /*
         * Prefer the conventional FAT32 partition types, then inspect any
         * remaining MBR entries.  The BPB is still validated, so a malformed
         * or unknown partition type cannot cause an arbitrary mount.
         */
        for (uint32_t pass = 0; pass < 2 && !found_volume; ++pass) {
            for (uint32_t partition = 0; partition < 4; ++partition) {
                uint8_t type = partition_type[partition];
                uint32_t start = partition_start[partition];
                uint32_t size = partition_size[partition];
                int conventional = type == 0x0B || type == 0x0C ||
                                    type == 0x1B || type == 0x1C;

                if ((pass == 0) != conventional || start == 0 || size == 0)
                    continue;
                if (device->sector_count != 0 &&
                    (start >= device->sector_count ||
                     size > device->sector_count - start))
                    continue;
                if (block_read_sector(device, start, fs->sector_buffer) != 0 ||
                    !looks_like_fat32_bpb(fs->sector_buffer,
                                          device->sector_size) ||
                    fat32(fs->sector_buffer + 32) > size ||
                    start > 0xFFFFFFFFu - fat32(fs->sector_buffer + 32))
                    continue;
                volume_start = start;
                volume_sectors = size;
                found_volume = 1;
                break;
            }
        }
    }
    if (!found_volume)
        return FAT32_EBADFS;

    fs->volume_start = volume_start;
    fs->bytes_per_sector = fat16(fs->sector_buffer + 11);
    fs->sectors_per_cluster = fs->sector_buffer[13];
    fs->reserved_sectors = fat16(fs->sector_buffer + 14);
    if (fs->bytes_per_sector != device->sector_size ||
        fs->bytes_per_sector < 512 || !power_of_two(fs->bytes_per_sector) ||
        fs->bytes_per_sector % 4 != 0 ||
        !power_of_two(fs->sectors_per_cluster) ||
        fs->sectors_per_cluster > 128 ||
        fs->reserved_sectors == 0 || fs->sector_buffer[16] == 0 ||
        fs->sector_buffer[16] > 2 ||
        fat16(fs->sector_buffer + 17) != 0 ||
        fat16(fs->sector_buffer + 19) != 0 ||
        fat16(fs->sector_buffer + 22) != 0 ||
        fat16(fs->sector_buffer + 42) != 0 ||
        (fs->sector_buffer[66] != 0x28 &&
         fs->sector_buffer[66] != 0x29))
        return FAT32_EBADFS;
    total_sectors = fat32(fs->sector_buffer + 32);
    fat_size = fat32(fs->sector_buffer + 36);
    fs->root_cluster = fat32(fs->sector_buffer + 44) & 0x0FFFFFFFu;
    if (total_sectors == 0 || fat_size == 0 ||
        (fat32(fs->sector_buffer + 44) & 0xF0000000u) != 0 ||
        fs->root_cluster < 2)
        return FAT32_EBADFS;
    if (volume_start > 0xFFFFFFFFu - total_sectors ||
        (volume_sectors != 0 && total_sectors > volume_sectors) ||
        (device->sector_count != 0 &&
         (volume_start > device->sector_count ||
          total_sectors > device->sector_count - volume_start)))
        return FAT32_EBADFS;
    data_start = (uint64_t)fs->reserved_sectors +
                 (uint64_t)fs->sector_buffer[16] * fat_size;
    if (data_start >= total_sectors ||
        data_start > 0xFFFFFFFFu)
        return FAT32_EBADFS;
    data_sectors = total_sectors - (uint32_t)data_start;
    clusters = data_sectors / fs->sectors_per_cluster;
    if (clusters == 0 ||
        clusters > 0x0FFFFFF5u ||
        fs->root_cluster - 2 >= clusters)
        return FAT32_EBADFS;
    fat_entries = (uint64_t)fat_size * fs->bytes_per_sector / 4;
    if (fat_entries < (uint64_t)clusters + 2)
        return FAT32_EBADFS;
    fs->fat_start = fs->reserved_sectors;
    fs->fat_sectors = fat_size;
    fs->data_start = (uint32_t)data_start;
    fs->cluster_count = clusters;
    fs->mounted = 1;
    return FAT32_OK;
}

fat32_result_t fat32_list_directory(fat32_fs_t *fs, const char *path,
                                    fat32_dir_callback_t callback, void *context)
{
    fat32_dirent_t entry;
    fat32_result_t result;
    uint32_t cluster;

    if (fs == (fat32_fs_t *)0 || callback == (fat32_dir_callback_t)0)
        return FAT32_EINVAL;
    if (!fs->mounted)
        return FAT32_EBADFS;
    result = lookup(fs, path == (const char *)0 ? "/" : path, &entry);
    if (result != FAT32_OK)
        return result;
    if ((entry.attributes & 0x10) == 0)
        return FAT32_ENOTDIR;
    cluster = entry.first_cluster;
    return scan_directory(fs, cluster, callback, context);
}

fat32_result_t fat32_read_file(fat32_fs_t *fs, const char *path,
                               uint8_t *buffer, uint32_t capacity,
                               uint32_t *bytes_read)
{
    fat32_dirent_t entry;
    fat32_result_t result;
    uint32_t cluster;
    uint32_t remaining;
    uint32_t copied = 0;
    uint32_t visited = 0;

    if (bytes_read == (uint32_t *)0 || fs == (fat32_fs_t *)0 ||
        path == (const char *)0)
        return FAT32_EINVAL;
    *bytes_read = 0;
    if (!fs->mounted)
        return FAT32_EBADFS;
    result = lookup(fs, path, &entry);
    if (result != FAT32_OK)
        return result;
    if ((entry.attributes & 0x10) != 0)
        return FAT32_EISDIR;
    if (entry.size != 0 && buffer == (uint8_t *)0)
        return FAT32_EINVAL;
    if (capacity < entry.size)
        return FAT32_ENOSPC;
    if (entry.size == 0) {
        *bytes_read = 0;
        return FAT32_OK;
    }
    cluster = entry.first_cluster;
    if (!valid_cluster(fs, cluster))
        return FAT32_EBADFS;
    remaining = entry.size;
    while (remaining != 0) {
        uint32_t sector_in_cluster;
        if (++visited > fs->cluster_count)
            return FAT32_ELOOP;
        for (sector_in_cluster = 0;
             sector_in_cluster < fs->sectors_per_cluster && remaining != 0;
             ++sector_in_cluster) {
            uint32_t sector = fs->data_start +
                              (cluster - 2) * fs->sectors_per_cluster +
                              sector_in_cluster;
            uint32_t amount = remaining < fs->bytes_per_sector
                                  ? remaining : fs->bytes_per_sector;
            result = read_sector(fs, sector);
            if (result != FAT32_OK)
                return result;
            for (uint32_t i = 0; i < amount; ++i)
                buffer[copied + i] = fs->sector_buffer[i];
            copied += amount;
            remaining -= amount;
        }
        if (remaining != 0) {
            result = next_cluster(fs, cluster, &cluster);
            if (result != FAT32_OK)
                return result;
            if (cluster == 0x0FFFFFFFu)
                return FAT32_EBADFS;
        }
    }
    *bytes_read = copied;
    return FAT32_OK;
}

const char *fat32_result_string(fat32_result_t result)
{
    switch (result) {
    case FAT32_OK: return "ok";
    case FAT32_EINVAL: return "invalid argument";
    case FAT32_EIO: return "disk I/O error";
    case FAT32_EBADFS: return "invalid FAT32 filesystem";
    case FAT32_ENOTFOUND: return "not found";
    case FAT32_ENOTDIR: return "not a directory";
    case FAT32_EISDIR: return "is a directory";
    case FAT32_EOVERFLOW: return "filesystem too large";
    case FAT32_ELOOP: return "cluster chain loop";
    case FAT32_ENOSPC: return "buffer too small";
    default: return "filesystem error";
    }
}
