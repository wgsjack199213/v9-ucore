#ifndef __KERN_SYNC_SYNC_H__
#define __KERN_SYNC_SYNC_H__
#include "u.h"
#include <defs.h>
#include <io.h>

bool
__intr_save(void) {
    return splhi();
}

void
__intr_restore(bool flag) {
    splx(flag);
}

#define local_intr_save(x)      do { x = __intr_save(); } while (0)
#define local_intr_restore(x)   __intr_restore(x);

#endif /* !__KERN_SYNC_SYNC_H__ */

