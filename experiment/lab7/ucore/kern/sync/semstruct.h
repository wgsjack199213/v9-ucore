#ifndef __KERN_SYNC_SEMSTRUCT_H__
#define __KERN_SYNC_SEMSTRUCT_H__
#include <wait.h>

typedef struct {
    int value;
    wait_queue_t wait_queue;
} semaphore_t;

#endif