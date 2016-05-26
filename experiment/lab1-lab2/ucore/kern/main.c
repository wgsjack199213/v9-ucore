#include <u.h>
#include <defs.h>
#include <list.h>
#include <atomic.h>
#include <string.h>
#include <io.h>
#include <sync.h>
#include <trap.h>
#include <default_pmm.h>
// #include <pmm.h>

void kern_init() {

    // extern char edata[], end[];

    // memset(edata, 0,
    //  - edata);

    // cons_init();                // init the console

    printf("(THU.CST) os is loading ...\n");

    pmm_init();                 // init physical memory management

    idt_init();                 // init interrupt descriptor table

    stmr(128*1024*1000);             // init clock interrupt

    asm(STI);                   // enable irq interrupt

    while (1) {
                                // do nothing
    }
}

main() {
    static int bss;     // last variable in bss segment
    endbss = &bss;
    kern_init();
}
