#include <defs.h>
#include <io.h>
#include <string.h>
#include <list.h>

/* [wikipedia]The simplest Page Replacement Algorithm(PRA) is a FIFO algorithm. The first-in, first-out
 * page replacement algorithm is a low-overhead algorithm that requires little book-keeping on
 * the part of the operating system. The idea is obvious from the name - the operating system
 * keeps track of all the pages in memory in a queue, with the most recent arrival at the back,
 * and the earliest arrival in front. When a page needs to be replaced, the page at the front
 * of the queue (the oldest page) is selected. While FIFO is cheap and intuitive, it performs
 * poorly in practical application. Thus, it is rarely used in its unmodified form. This
 * algorithm experiences Belady's anomaly.
 *
 * Details of FIFO PRA
 * (1) Prepare: In order to implement FIFO PRA, we should manage all swappable pages, so we can
 *              link these pages into pra_list_head according the time order. At first you should
 *              be familiar to the struct list in list.h. struct list is a simple doubly linked list
 *              implementation. You should know howto USE: list_init, list_add(list_add_after),
 *              list_add_before, list_del, list_next, list_prev. Another tricky method is to transform
 *              a general list struct to a special struct (such as struct page). You can find some MACRO:
 *              le2page (in memlayout.h), (in future labs: le2vma (in vmm.h), le2proc (in proc.h),etc.
 */

list_entry_t pra_list_head;

//list_entry_t pra_clock_hand;
/*
 * (2) _fifo_init_mm: init pra_list_head and let  mm->sm_priv point to the addr of pra_list_head.
 *              Now, From the memory control struct mm_struct, we can access FIFO PRA
 */
int _fifo_init_mm(struct mm_struct *mm)
{
  list_init(&pra_list_head);
  mm->sm_priv = &pra_list_head;
  mm->clock_hand = &pra_list_head;
  //cprintf(" mm->sm_priv %x in fifo_init_mm\n",mm->sm_priv);
  return 0;
}
/*
 * (3)_fifo_map_swappable: According FIFO PRA, we should link the most recent arrival page at the back of pra_list_head qeueue
 */
int _fifo_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
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
 *  (4)_fifo_swap_out_victim: According FIFO PRA, we should unlink the  earliest arrival page in front of pra_list_head qeueue,
 *                            then set the addr of addr of this page to ptr_page.
 */
int
_fifo_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick)
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
_fifo_check_swap(void) {
  int tmp;
  printf("Read Virt Page c in fifo_check_swap\n");
  tmp = *(unsigned char *)0x3000;
  assert(pgfault_num==4);
  printf("write Virt Page a in fifo_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num==4);
  printf("read Virt Page d in fifo_check_swap\n");
  tmp = *(unsigned char *)0x4000;
  assert(pgfault_num==4);
  printf("write Virt Page b in fifo_check_swap\n");
  *(unsigned char *)0x2000 = 0x0b;
  assert(pgfault_num==4);
  printf("read Virt Page e in fifo_check_swap\n");
  tmp = *(unsigned char *)0x5000;
  assert(pgfault_num==5);
  printf("read Virt Page b in fifo_check_swap\n");
  tmp = *(unsigned char *)0x2000;
  assert(pgfault_num==5);
  printf("write Virt Page a in fifo_check_swap\n");
  *(unsigned char *)0x1000 = 0x0a;
  assert(pgfault_num==5);
  printf("read Virt Page b in fifo_check_swap\n");
  tmp = *(unsigned char *)0x2000;
  assert(pgfault_num==5);
  printf("read Virt Page c in fifo_check_swap\n");
  tmp = *(unsigned char *)0x3000;
  assert(pgfault_num==5);
  printf("read Virt Page d in fifo_check_swap\n");
  tmp = *(unsigned char *)0x4000;
  //assert(pgfault_num==9);
  printf("write Virt Page e in fifo_check_swap\n");
  *(unsigned char *)0x5000 = 0x0e;
  //assert(pgfault_num==10);
  printf("write Virt Page a in fifo_check_swap\n");
  //assert(*(unsigned char *)0x1000 == 0x0a);
  *(unsigned char *)0x1000 = 0x0a;
  //assert(pgfault_num==11);
  return 0;
}


int _fifo_init(void) {
  return 0;
}

int _fifo_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
  return 0;
}

int _fifo_tick_event(struct mm_struct *mm) {
  return 0;
}
