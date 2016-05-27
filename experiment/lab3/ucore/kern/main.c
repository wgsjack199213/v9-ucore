#include <u.h>
#include <defs.h>
#include <list.h>
#include <atomic.h>
#include <string.h>
#include <io.h>
#include <sync.h>
#include <default_pmm.h>
#include <trap.h>
// #include <fs.h>
// #include <swap_fifo.h>
#include <swap.h>

void kern_init() {
  printf("(THU.CST) os is loading ...\n");

  pmm_init();                   // init physical memory management

  idt_init();                   // init interrupt descriptor table

  asm (STI);                    // enable irq interrupt

  tlb_clear_enable = 1;

  spage(1);

  vmm_init();                   // init virtual memory management

  swap_init();                  // init swap

  stmr(128*1024*1000);          // init clock interrupt

  while (1) {
    // do nothing
  }
}

main() {
  int *ksp;                // temp kernel stack pointer

  static int bss;       // last variable in bss segment
  endbss = &bss;

  ksp = ((uint)kstack + sizeof(kstack) - 8) & -8;
  asm (LL, 4);
  asm (SSP);

  kern_init();

  asm (HALT);
}
