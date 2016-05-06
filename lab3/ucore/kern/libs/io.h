#ifndef __LIBS_IO_H__
#define __LIBS_IO_H__
#include "u.h"
#include <defs.h>

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
cout(char c) {
  out(1, c);
}

//输出数字
printn(int n) {
  if (n > 9) { printn(n / 10); n %= 10; }
  cout(n + '0');
}

//输出16进制数字
printx(uint n) {
  if (n > 15) { printx(n >> 4); n &= 15; }
  cout(n + (n > 9 ? 'a' - 10 : '0'));
}

//输出函数printf
printf(char *f, ...) {
  int n, e = splhi();
  char *s;
  va_list v;
  va_start(v, f);
  while (*f) {
    if (*f != '%') { cout(*f++); continue; }
    switch (*++f) {
      case 'd': f++; if ((n = va_arg(v,int)) < 0) { cout('-'); printn(-n); } else printn(n); continue;
      case 'x': f++; printx(va_arg(v,int)); continue;
      case 's': f++; for (s = va_arg(v, char *); *s; s++) cout(*s); continue;
    }
    cout('%');
  }
  splx(e);
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