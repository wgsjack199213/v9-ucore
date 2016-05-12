#ifndef __LIBS_ATOMIC_H__
#define __LIBS_ATOMIC_H__

set_bit(int nr, uint* addr) {
    *addr = (*addr) | (1 << nr);
}

clear_bit(int nr, uint* addr) {
    *addr = ((*addr) | (1 << nr)) ^ (1 << nr);
}

change_bit(int nr, uint* addr) {
    *addr = (*addr) ^ (1 << nr);
}

int test_bit(int nr, uint* addr) {
    if (((*addr) & (1 << nr)) == 0)
        return 0;
    return 1;
}

#endif /* !__LIBS_ATOMIC_H__ */