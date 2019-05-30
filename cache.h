#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H
#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/disk.h"
#include "filesys/filesys.h"

struct cache{
    uint8_t *data;
    disk_sector_t sector;
    bool dirty;
    bool used;
};

void cache_init(void);
void evict(void);
int cache_insert(disk_sector_t sector);
void cache_delete(disk_sector_t sector);
void cache_done(void);
void cache_refresh(void);
void cache_read(disk_sector_t sector, uint8_t *buffer, off_t size, off_t offset);
void cache_write(disk_sector_t sector, const uint8_t *buffer, off_t size, off_t offset);
#endif /* filesys/cache.h */
