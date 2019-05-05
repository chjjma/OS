#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "threads/vaddr.h"

struct sup_page_table_entry 
{
	uint32_t* user_vaddr;
	uint32_t* phys_addr;
        uint32_t* prev;
	uint32_t* next;
	uint32_t page_no;
	uint64_t access_time;
	short loc; //loc=0:all-zero page, loc=1:frame, loc=2:swap, loc=3:disk

	bool read_only;
	bool dirty;
	bool accessed;
};

void page_init (void);
struct sup_page_table_entry *allocate_page (void *addr);
struct sup_page_table_entry *allocaate_read_only_page (void *addr);
struct sup_page_table_entry *find_page(void *addr);
struct void change_spte_paddr(struct sup_page_table_entry *spte, void *addr);
struct void change_spte_time(struct sup_page_table_entry *spte, uint64_t time);
struct void change_spte_loc(struct sup_page_table_entry *spte, short loc);
struct void spte_dirtied(struct sup_page_table_entry *spte);
struct void spte_accessed(struct sup_page_table_entry *spte);
struct void destroy_page_entry(struct sup_page_table_entry *spte);
struct void destroy_page_addr(void *addr);
struct void destroy_page();

static inline unsigned pg_no (const void *va){
  return (uintptr_t) va >> PGBITS;
}

static inline unsigned convert_to_pg (const void *va){
  return ((uintptr_t) (va) >> PGBITS) << PGBITS;
}

#endif /* vm/page.h */
