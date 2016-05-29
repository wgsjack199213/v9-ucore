#ifndef __LIBS_ATOMIC_H__
#define __LIBS_ATOMIC_H__
#include <v9.h>

set_bit(int nr, uint* addr) {
  int e = splhi();
  *addr = (*addr) | (1 << nr);
  splx(e);
}

clear_bit(int nr, uint* addr) {
  int e = splhi();
  *addr = ((*addr) | (1 << nr)) ^ (1 << nr);
  splx(e);
}

change_bit(int nr, uint* addr) {
  int e = splhi();
  *addr = (*addr) ^ (1 << nr);
  splx(e);
}

int test_bit(int nr, uint* addr) {
  int e = splhi();
  if (((*addr) & (1 << nr)) == 0) {
    splx(e);
    return 0;
  }
  splx(e);
  return 1;
}

#endif /* !__LIBS_ATOMIC_H__ */
