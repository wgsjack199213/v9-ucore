#include <defs.h>
#include <io.h>
#include <string.h>
#include <list.h>

//
// Enhanced Clock Algorithm
//


list_entry_t pra_list_head;

//list_entry_t pra_clock_hand;
/*
 * (2) _enhclock_init_mm: init pra_list_head and let  mm->sm_priv point to the addr of pra_list_head.
 *              Now, From the memory control struct mm_struct, we can access FIFO PRA
 */
int _enhclock_init_mm(struct mm_struct *mm)
{
  list_init(&pra_list_head);
  mm->sm_priv = &pra_list_head;
  mm->clock_hand = &pra_list_head;
  //cprintf(" mm->sm_priv %x in enhclock_init_mm\n",mm->sm_priv);
  return 0;
}
/*
 * (3)_enhclock_map_swappable: According FIFO PRA, we should link the most recent arrival page at the back of pra_list_head qeueue
 */
int _enhclock_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
{
  list_entry_t *head=(list_entry_t*) mm->sm_priv;
  list_entry_t *entry=&(page->pra_page_link); 
  
  assert(entry != NULL && head != NULL);
  //record the page access situlation
  /*LAB3 EXERCISE 2: YOUR CODE*/
  //(1)link the most recent arrival page at the back of the pra_list_head qeueue.
  list_add(head, entry);
  return 0;
}

/*
 *  (4)_enhclock_swap_out_victim: According FIFO PRA, we should unlink the  earliest arrival page in front of pra_list_head qeueue,
 *                            then set the addr of addr of this page to ptr_page.
 */
int
_enhclock_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
{
  struct Page *p;
  	list_entry_t *head = (list_entry_t*) mm->sm_priv;
  	list_entry_t *hand = (list_entry_t*) mm->clock_hand;
	pte_t *ptep; 
  	int pte_a;
	int pte_d;
  	
	if (hand == head) {
	  	hand = head->next;
	}
  
  assert(hand != NULL);
  assert(in_tick==0);
  /* Select the victim */
  /*LAB3 EXERCISE 2: YOUR CODE*/
  //(1)  unlink the  earliest arrival page in front of pra_list_head qeueue
  ///(2)  set the addr of addr of this page to ptr_page
  /* Select the tail */
  //le = head->prev;

  //assert(head!=le);
  
  	while(1) {
  		p = le2page(hand, pra_page_link);
  		//p = le2page(hand, pra_page_link);
		//ptep = get_pte(mm->pgdir, page2kva(p), 0);
		ptep = get_pte(mm->pgdir, p->pra_vaddr, 0);
		pte_a = (*ptep >> 5) % 2;
		pte_d = (*ptep >> 6) % 2;
  		printf("----------ptep PTE_A and PTE_D: %d %d\n", pte_a, pte_d);
		//break;
		if (pte_a == 0 && pte_d == 0) {
			break;
		} else if (pte_a == 0 && pte_d == 1) {
			*ptep -= (1 << 6);
		} else { 
			*ptep -= (1 << 5);
		}
		hand = hand->next;
		if (hand == head) {
			hand = hand->next;
		}
	}
	
  	printf("----------Replacement chosen!\n");
	mm->clock_hand = hand->next;
	
	//list_del(hand);
	list_del(hand);

  //list_del(le);
  assert(p !=NULL);
  *ptr_page = p;
  printf("----------Here!\n");
  return 0;
}

int
_enhclock_check_swap(void) {
  int tmp;
  printf("Read Virt Page c in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x3000;
  assert(pgfault_num==4);
  printf("write Virt Page a in enhclock_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num==4);
  printf("read Virt Page d in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x4000;
  assert(pgfault_num==4);
  printf("write Virt Page b in enhclock_check_swap\n");
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num==4);
  printf("read Virt Page e in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x5000;
  assert(pgfault_num==5);
  printf("read Virt Page b in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x2000;
  assert(pgfault_num==5);
  printf("write Virt Page a in enhclock_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num==5);
  printf("read Virt Page b in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x2000;
  assert(pgfault_num==5);
  printf("read Virt Page c in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x3000;
  assert(pgfault_num==5);
  printf("read Virt Page d in enhclock_check_swap\n");
  tmp = *(unsigned char *)0x4000;
  assert(pgfault_num==6);
  printf("write Virt Page e in enhclock_check_swap\n");
  *(unsigned char *)0x5000 = 0x0e;
  assert(pgfault_num==6);
  printf("write Virt Page a in enhclock_check_swap\n");
  assert(*(unsigned char *)0x1000 == 0x0a);
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num==6);
  return 0;
}


int _enhclock_init(void) {
  return 0;
}

int _enhclock_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
  return 0;
}

int _enhclock_tick_event(struct mm_struct *mm) {
  return 0;
}
