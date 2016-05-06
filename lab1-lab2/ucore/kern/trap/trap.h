#ifndef __KERN_TRAP_TRAP_H__
#define __KERN_TRAP_TRAP_H__
#include <defs.h>

struct pushregs {
    int sp, pad1;
    double g;
    double f;
    int c,  pad2;
    int b,  pad3;
    int a,  pad4;
};

struct trapframe {
    struct pushregs tf_regs;
    int fc, pad5;
    int pc, pad6;
};

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

void print_regs(struct pushregs *regs) {
    printf("  SP : 0x%x\n", regs -> sp);
    printf("  REG_G : %f\n", regs -> g);
    printf("  REG_F : %f\n", regs -> f);
    printf("  REG_A : 0x%x\n", regs -> a);
    printf("  REG_B : 0x%x\n", regs -> b);
    printf("  REG_C : 0x%x\n", regs -> c);
}

void print_trapframe(struct trapframe *tf) {
    printf("trapframe at %x\n", tf);
    print_regs(&tf->tf_regs);
    if (!trap_in_kernel(tf)) {
      printf("Trap in usermode!\n");
    }else{
      printf("Trap in kernel!\n");
    }
    printf("Error Code: %d\n", tf->fc);
    printf("PC : 0x%x\n", tf->pc);
    printf("\n");
}

void trap_dispatch(struct trapframe *tf)
{
  uint va;
  print_trapframe(tf);
  switch (tf -> fc) {
    case FSYS: panic("FSYS from kernel");
    case FSYS + USER:
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