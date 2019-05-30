#include "filesys/cache.h"
#include <list.h>
#include <debug.h>
#include <string.h>
#include "filesys/filesys.h"
#include "devices/timer.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "threads/thread.h"

/* The Cache */
struct cache buffer_cache[64];

/*write index of current cache*/
static int wi;

/* eviction index of current cache */
static int di;

/*protects whole cache buffer*/
static struct lock cache_lock;

/* eviction needed?*/
static bool full; 

//initializing cache
void
cache_init (void){
    int i;
    for(i=0;i<64;i++){
        buffer_cache[i].data=malloc(DISK_SECTOR_SIZE);
	buffer_cache[i].dirty=false;
	buffer_cache[i].used=false;
	buffer_cache[i].sector=-1;
    }
    di=0;
    wi=0;
    lock_init(&cache_lock);
    full=false;
    thread_create("cache_refresh", PRI_MAX, cache_refresh, NULL);
}

void
cache_evict(void){
    ASSERT(full==true);
    cache_delete(buffer_cache[di].sector);
}

int
cache_insert(disk_sector_t sector){
    ASSERT(sector!=-1);
    int t;
	for(t=0;t<64;t++)
	    if(buffer_cache[t].sector==sector)
		return t;
	if(full) cache_evict();
	disk_read(filesys_disk, sector, buffer_cache[wi].data);
	t=wi;
	buffer_cache[wi].used=true;
	buffer_cache[wi].sector=sector;
	wi+=1;
	wi%=64;
	if(di==wi)full=true;
    return t;
}

void
cache_delete(disk_sector_t sector){
    ASSERT(sector!=-1);
    int i;
    for(i=0;i<64;i++)if(buffer_cache[i].sector==sector) break;
    ASSERT(i!=64);
    if(buffer_cache[i].dirty)
	disk_write(filesys_disk, sector, buffer_cache[i].data);
	full=false;
	buffer_cache[i].dirty=false;
	buffer_cache[i].used=false;
	buffer_cache[i].sector=-1;
	di=i+1;
	di%=64;
}

void
cache_done(void){
    //lock_acquire(&cache_lock);
    int i;
    for(i=0;i<64;i++){
	if(buffer_cache[i].dirty) disk_write(filesys_disk, buffer_cache[i].sector, buffer_cache[i].data);
	free(buffer_cache[i].data);
    }
    wi=0;
    di=0;
    full=false;
    //lock_release(&cache_lock);
}

void
cache_refresh(void){
    int i;
    for(;;){
	timer_sleep(TIMER_FREQ);
	lock_acquire(&cache_lock);
    	    for(i=0;i<64;i++)
		if(buffer_cache[i].dirty){
	    	    disk_write(filesys_disk, buffer_cache[i].sector, buffer_cache[i].data);
	    	    buffer_cache[i].dirty=false;
		}
	lock_release(&cache_lock);
    }
}

void
cache_read(disk_sector_t sector, uint8_t *buffer, off_t size, off_t offset){
    //printf("cache read:in sector %d to buffer with size %d in offset %d\n",sector,size,offset);
    lock_acquire(&cache_lock);
    int i;
    for(i=0;i<64;i++)if(buffer_cache[i].sector==sector)break;
    if(i==64) i=cache_insert(sector);
    memcpy(buffer, buffer_cache[i].data+offset, size);
    //printf("offset %d reading done\n", offset);
    lock_release(&cache_lock);
}


void
cache_write(disk_sector_t sector, const uint8_t *buffer, off_t size, off_t offset){
    int i;
    lock_acquire(&cache_lock);
    for(i=0;i<64;i++)if(buffer_cache[i].sector==sector)break;
    if(i==64) i=cache_insert(sector);	
    memcpy(buffer_cache[i].data+offset, buffer, size);
    buffer_cache[i].dirty=true;
    lock_release(&cache_lock);
}


