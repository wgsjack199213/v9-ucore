#ifndef __KERN_SYNC_SEM_H__
#define __KERN_SYNC_SEM_H__

#include <defs.h>
#include <wait.h>
#include <atomic.h>
#include <kmalloc.h>
#include <proc.h>
#include <sync.h>
#include <semstruct.h>

void sem_init(semaphore_t *sem, int value);
void up(semaphore_t *sem);
void down(semaphore_t *sem);
bool try_down(semaphore_t *sem);

void
sem_init(semaphore_t *sem, int value) {
  sem->value = value;
  wait_queue_init(&(sem->wait_queue));
}

static void __up(semaphore_t *sem, uint32_t wait_state) {
  bool intr_flag;
  wait_t *wait;
  local_intr_save(intr_flag);
  {
    if ((wait = wait_queue_first(&(sem->wait_queue))) == NULL) {
      sem->value++;
    }
    else {
      assert(wait->proc->wait_state == wait_state);
      wakeup_wait(&(sem->wait_queue), wait, wait_state, 1);
    }
  }
  local_intr_restore(intr_flag);
}

static uint32_t __down(semaphore_t *sem, uint32_t wait_state) {
  bool intr_flag;
  wait_t __wait, *wait;
  local_intr_save(intr_flag);
  if (sem->value > 0) {
    sem->value--;
    local_intr_restore(intr_flag);
    return 0;
  }
  wait = &__wait;
  wait_current_set(&(sem->wait_queue), wait, wait_state);
  local_intr_restore(intr_flag);

  schedule();

  local_intr_save(intr_flag);
  wait_current_del(&(sem->wait_queue), wait);
  local_intr_restore(intr_flag);

  if (wait->wakeup_flags != wait_state) {
    return wait->wakeup_flags;
  }
  return 0;
}

void
up(semaphore_t *sem) {
  __up(sem, WT_KSEM);
}

void
down(semaphore_t *sem) {
  uint32_t flags = __down(sem, WT_KSEM);
  assert(flags == 0);
}

bool
try_down(semaphore_t *sem) {
  bool intr_flag, ret = 0;
  local_intr_save(intr_flag);
  if (sem->value > 0) {
    sem->value--, ret = 1;
  }
  local_intr_restore(intr_flag);
  return ret;
}

#endif /* !__KERN_SYNC_SEM_H__ */
