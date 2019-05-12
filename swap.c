#include "vm/swap.h"
#include "vm/page.h"
#include "devices/disk.h"
#include "threads/synch.h"
#include <bitmap.h>


/* The swap device */
static struct disk *swap_device;

/* Tracks in-use and free swap slots */
static struct bitmap *swap_table;

/* Protects swap_table */
static struct lock swap_lock;

/* 
 * Initialize swap_device, swap_table, and swap_lock.
 */

int swapsize; 

void 
swap_init (void)
{
  swap_device = disk_get(1,1);
  swapsize = DISK_SECTOR_SIZE*disk_size(swap_device)/PGSIZE;
  swap_table = bitmap_create(disk_size(swapsize));
  lock_init(swap_lock);
}

/*
 * Reclaim a frame from swap device.
 * 1. Check that the page has been already evicted. 
 * 2. You will want to evict an already existing frame
 * to make space to read from the disk to cache. 
 * 3. Re-link the new frame with the corresponding supplementary
 * page table entry. 
 * 4. Do NOT create a new supplementray page table entry. Use the 
 * already existing one. 
 * 5. Use helper function read_from_disk in order to read the contents
 * of the disk into the frame. 
 */ 
bool 
swap_in (int index, uint8_t *frame)
{
  lock_acquire(swap_lock);
  ASSERT(index>=0);
  if(swap_table[index]==false) {
	return false;	
  }
  lock_release(swap_lock);
  read_from_disk(frame, index);
  lock_acquire(swap_lock);
	swap_table[index]=false;
  lock_release(swap_lock);
  return true;
}

/* 
 * Evict a frame to swap device. 
 * 1. Choose the frame you want to evict. 
 * (Ex. Least Recently Used policy -> Compare the timestamps when each 
 * frame is last accessed)
 * 2. Evict the frame. Unlink the frame from the supplementray page table entry
 * Remove the frame from the frame table after freeing the frame with
 * pagedir_clear_page. 
 * 3. Do NOT delete the supplementary page table entry. The process
 * should have the illusion that they still have the page allocated to
 * them. 
 * 4. Find a free block to write you data. Use swap table to get track
 * of in-use and free swap slots.
 */
bool
swap_out (uint8_t *frame, struct sup_page_table_entry *spte)
{
  if(frame==NULL) return false;
  lock_acquire(swap_lock);
  int i;
  for(i=0;i<swapsize;i++){
  	if(swap_table[i]==false) break;
  }
  if (i==swapsize) {
  	ASSERT(0);
  	PANIC();
  }
  swap_table[i]=true;
  write_to_disk(frame,i);
  spte->swap_index=i;
  lock_release(swap_lock);
  return true;
}

/* 
 * Read data from swap device to frame. 
 * Look at device/disk.c
 */
void read_from_disk (uint8_t *frame, int index)
{
  disk_read(swap_disk, 4*index, frame);
  disk_read(swap_disk, 4*index+1, frame+DISK_SECTOR_SIZE);
  disk_read(swap_disk, 4*index+2, frame+2*DISK_SECTOR_SIZE);
  disk_read(swap_disk, 4*index+3, frame+3*DISK_SECTOR_SIZE);
}

/* Write data to swap device from frame */
void write_to_disk (uint8_t *frame, int index)
{
  disk_write(swap_disk, 4*index, frame);
  disk_write(swap_disk, 4*index+1, frame+DISK_SECTOR_SIZE);
  disk_write(swap_disk, 4*index+2, frame+2*DISK_SECTOR_SIZE);
  disk_write(swap_disk, 4*index+3, frame+3*DISK_SECTOR_SIZE);

}
