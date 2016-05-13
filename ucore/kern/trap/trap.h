#ifndef __KERN_TRAP_TRAP_H__
#define __KERN_TRAP_TRAP_H__
#include <defs.h>
#include <syscall.h>
#include <tfstruct.h>

enum {    // processor fault codes
    FMEM,   // bad physical address
    FTIMER, // timer interrupt
    FKEYBD, // keyboard interrupt
    FPRIV,  // privileged instruction
    FINST,  // illegal instruction
    FSYS,   // software trap
    FARITH, // arithmetic trap
    FIPAGE, // page fault on opcode fetch
    FWPAGE, // page fault on write
    FRPAGE, // page fault on read
    USER=16 // user mode exception
};

uint ticks;

void trapname(int fc) {
  switch (fc) {
    case FMEM:
    case FMEM + USER:
      printf("bad physical address\n");
      return;
    case FTIMER:
    case FTIMER + USER:
      printf("timer interrupt\n");
      return;
    case FKEYBD:
    case FKEYBD + USER:
      printf("keyboard interrupt");
      return;
    case FPRIV:
    case FPRIV + USER:
      printf("privileged instruction");
      return;
    case FINST:
    case FINST + USER:
      printf("illegal instruction");
      return;
    case FSYS:
    case FSYS + USER:
      printf("software trap");
      return;
    case FARITH:
    case FARITH + USER:
      printf("arithmetic trap");
      return;
    case FIPAGE:
    case FIPAGE + USER:
      printf("page fault on opcode fetch");
      return;
    case FWPAGE:
    case FWPAGE + USER:
      printf("page fault on write");
      return;
    case FRPAGE:
    case FRPAGE + USER:
      printf("page fault on read");
      return;
    default:
      printf("Unkonw!\n");
  }
}

uint trap_in_kernel(struct trapframe *tf) {
  if (tf->fc >= USER)
    return 0;
  else
    return 1;
}

void print_pgfault(struct trapframe *tf) {
    /* error_code:
     * bit 0 == 0 means no page found, 1 means protection fault
     * bit 1 == 0 means read, 1 means write
     * bit 2 == 0 means kernel, 1 means user
     * */
    printf("page fault at 0x%x: %c/%c [%s].\n", lvadr(),
            (tf->fc >= USER) ? 'U' : 'K',
            (tf->fc == FWPAGE || tf->fc == USER + FWPAGE) ? 'W' : 'R',
            (tf->tf_regs.b & 1) ? "protection fault" : "no page found");
}

int pgfault_handler(struct trapframe *tf) {
    print_pgfault(tf);
    if (check_mm_struct != NULL) {
        return do_pgfault(check_mm_struct, tf->fc, lvadr());
    }
    panic("unhandled page fault.\n");
}


void trap_dispatch(struct trapframe *tf)
{
  uint va;
  uint ret;
  print_trapframe(tf);
  switch (tf -> fc) {
    case FSYS: panic("FSYS from kernel");
    case FSYS + USER:
      tf->tf_regs.a = syscall(tf);
      return;
    case FMEM:
      panic("FMEM from kernel");
    case FMEM   + USER:
      printf("FMEM + USER\n");
      return;
      // exit(-1);
    case FPRIV:
      panic("FPRIV from kernel");
    case FPRIV  + USER:
      printf("FPRIV + USER\n");
      return;
      //exit(-1);
    case FINST:
      panic("FINST from kernel");
    case FINST  + USER:
      printf("FINST + USER\n");
      return;
      //exit(-1);
    case FARITH:
      panic("FARITH from kernel");
    case FARITH + USER:
      printf("FARITH + USER\n");
      return;
      //exit(-1); // XXX psignal(SIGFPT)
    case FIPAGE:
      printf("FIPAGE from kernel [0x%x]", lvadr());
      panic("!\n");
    case FIPAGE + USER:
      printf("FIPAGE + USER [0x%x]", lvadr());
      return;
      //exit(-1); // XXX psignal(SIGSEG) or page in
    case FWPAGE:
    case FWPAGE + USER:
    case FRPAGE:
    case FRPAGE + USER:
      if ((ret = pgfault_handler(tf)) != 0) {
            print_trapframe(tf);
            panic("handle pgfault failed. %e\n", ret);
      }
      tf->pc = (uint *)(tf->pc) - 1;
      spage(1);
      return;
    case FTIMER:
    case FTIMER + USER:
      ticks = ticks + 1;
      printf("%x\n", ticks);
      return;
    case FKEYBD:
    case FKEYBD + USER:
      //consoleintr();
      return;
  }
}

void trap(uint *sp, double g, double f, int c, int b, int a, int fc, uint *pc) {
  trap_dispatch(&sp);
}

void alltraps()
{
  asm(PSHA);
  asm(PSHB);
  asm(PSHC);
  asm(PSHF);
  asm(PSHG);
  asm(LUSP); asm(PSHA);
  trap();
  asm(POPA); asm(SUSP);
  asm(POPG);
  asm(POPF);
  asm(POPC);
  asm(POPB);
  asm(POPA);
  asm(RTI);
}

void idt_init() {
  ivec(alltraps);
}

#endif /* !__KERN_TRAP_TRAP_H__ */
