#ifndef __LIBS_IO_H__
#define __LIBS_IO_H__
#include "u.h"
#include <defs.h>
#include <printfmt.h>

int in(port)    { asm(LL,8); asm(BIN); }                    //从port获得输入
out(port, val)  { asm(LL,8); asm(LBL,16); asm(BOUT); }      //输出val到port
ivec(void *isr) { asm(LL,8); asm(IVEC); }                   //设置中断向量 <- isr
lvadr()         { asm(LVAD); }                              //通过LVAD指令可获得访问异常的虚地址并赋值给寄存器a
uint msiz()     { asm(MSIZ); }                              //获取内存大小
stmr(val)       { asm(LL,8); asm(TIME); }                   //设置timeout <- val
pdir(val)       { asm(LL,8); asm(PDIR); }                   //设置PTBR <- val
spage(val)      { asm(LL,8); asm(SPAG); }                   //设置页机制开关 <- val
splhi()         { asm(CLI); }                               //屏蔽中断
splx(int e)     { if (e) asm(STI); }                        //屏蔽使能

// console device
//输出 understand
static void cout(char c) {
  out(1, c);
}

static void cputch(int c, int *cnt, int unused) {
    cout(c);
    (*cnt) ++;
}

/* cputchar - writes a single character to stdout */
void
cputchar(int c) {
    cout(c);
}

static int
vkprintf(char *fmt, va_list ap) {
    int cnt = 0;
    int NO_FD = -0x9527; //TODO add unistd.h and use #define
    vprintfmt(cputch, NO_FD, &cnt, fmt, ap);
    return cnt;
}

//输出函数printf
int printf(char *fmt, ...) {
    va_list ap;
    int cnt;
    va_start(ap, fmt);
    cnt = vkprintf(fmt, ap);
    // va_end(ap);
    return cnt;
}

panic(char *s)  //异常输出panic
{
  asm(CLI);
  out(1,'p'); out(1,'a'); out(1,'n'); out(1,'i'); out(1,'c'); out(1,':'); out(1,' ');
  while (*s) out(1,*s++);
  out(1,'\n');
  asm(HALT);
}

assert(int x) {
    if (x == 0)
        panic("assertion failed: %s", x);
}

#endif /* !__LIBS_IO_H__ */
