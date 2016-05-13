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

/* *
 * test_and_set_bit - Atomically set a bit and return its old value
 * @nr:     the bit to set
 * @addr:   the address to count from
 * */
bool
test_and_set_bit(int nr, uint* addr) {
    int oldbit = ((*addr) & (1 << nr));
    *addr = (*addr) | (1 << nr);
    return oldbit != 0;
}

/* *
 * test_and_clear_bit - Atomically clear a bit and return its old value
 * @nr:     the bit to clear
 * @addr:   the address to count from
 * */
bool
test_and_clear_bit(int nr, uint* addr) {
    int oldbit = ((*addr) & (1 << nr));
    *addr = ((*addr) | (1 << nr)) ^ (1 << nr);
    return oldbit != 0;
}


#endif /* !__LIBS_ATOMIC_H__ */