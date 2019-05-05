#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/init.h"
/*
 * Initialize supplementary page table
 */
struct sup_page_table_entry *spt_head;
struct sup_page_table_entry *spt_tail;
page_init (void){
    //initialize head
    spt_head=malloc(size_t(struct sup_page_table_entry));
    spt_head.user_vaddr=NULL;
    spt_head.phys_addr=NULL;
    spt_head.prev=NULL;
    spt_head.next=NULL;
    spt_head.page_no=0;
    spt_head.access_time=0;
    spt_head.loc=1;
    spt_head.read_only=false;
    spt_head.dirty=false;
    spt_head.accessed=false;
    spt_tail=spt_head;
}

/*
 * Make new supplementary page table entry for page containing virtual address addr 
 */
struct sup_page_table_entry *
allocate_page (void *addr){
    ASSERT(is_user_vaddr(addr));//page not in virtual user address
    addr=convert_to_pg(addr);
    struct sup_page_table_entry *spt_tmp;
    //initializing
    spt_tmp=malloc(size_t(struct sup_page_table_entry));
    spt_tmp.user_vaddr=addr;
    spt_tmp.phys_addr=NULL;
    spt_tmp.prev=spt_tail;
    spt_tmp.next=NULL;
    spt_tmp.page_no=pg_no(addr);
    spt_tmp.access_time=0;
    spt_tmp.loc=1;
    spt_tmp.read_only=false;
    spt_tmp.dirty=false;
    spt_tmp.accesssed=false;
    //adds new entry into table
    spt_tail.next=spt_tmp;
    spt_tail=spt_tmp;
    return spt_tmp;
}

/*
 *Make new supplementary page table entry for read-only page containing virtual address addr
 */
struct sup_page_table_entry *
allocate_read_only_page (void *addr){
    sup_page_table_entry *spt_tmp=allocate_page(addr);
    spt_tmp.read_only=true;
}

/*
 * Find supplementary page table entry of page containing virtual address addr
 */
struct sup_page_table_entry *
find_page (void *addr){
    addr=convert_to_pg(addr);
    struct sup_page_table_entry *spt_tmp=sup_head.next;
    //searching
    while(sup_tmp!=NULL){
	if(sup_tmp.user_vaddr==addr)break;
	sup_tmp=sup_tmp.next;
    }
    return sup_tmp;
}

/*
 * change phys_addr of spte to addr
*/
struct void
change_spte_paddr(struct sup_page_table_entry *spte, void *addr){
    spte.phys_addr=addr;
}

/*
 * change access_time of spte to time
*/
struct void
change_spte_time(struct sup_page_table_entry *spte, uint64_t time){
    spte.access_time=time;
}

/*
 * location of page of spte changed to loc (>0:in frame, =0:in swap, <0:in disk)
*/
struct void
change_spte_loc(struct sup_page_table_entry *spte, short loc){
    spte.loc=loc;
}

/*
 * change spte dirty to true
 */
struct void
spte_dirtied(struct sup_page_table_entry *spte){
    spte.dirty=true;
}

/*
 * change spte accessed to true
 */
struct void
spte_accessed(struct sup_page_table_entry *spte){
    spte.accessed=true;
}

/*
 * Destroy supplementary page table entry in address spte
 */
struct void
destroy_page_entry (struct sup_page_table_entry *spte){
    if(spte_prev!=NULL)spte_prev.next=spte_next;
    if(spte_next!=NULL)spte_next.prev=spte_prev;
    free(spte);
}

/*
 * Destroy supplementary page table entry of page in virtual address addr
 */
struct void
destroy_page_addr (void *addr){
    addr=convert_to_pg(addr);
    struct sup_page_table_entry *sup_addr=find_page(addr);
    destroy_page_entry(sup_tmp);
}

struct void
destroy_page (){
    struct sup_page_table_entry *sup_tmp=sup_head;
    while(sup_head!=NULL){
	sup_tmp=sup_head;
	sup_head=sup_head.next;
	free(sup_tmp);
    }
}
