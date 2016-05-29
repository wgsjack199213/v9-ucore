#ifndef __KERN_SYNC_WAIT_H__
#define __KERN_SYNC_WAIT_H__

#include <defs.h>
#include <list.h>
#include <pstruct.h>
#include <sync.h>

typedef struct {
  list_entry_t wait_head;
} wait_queue_t;

struct proc_struct;

typedef struct {
  struct proc_struct *proc;
  uint32_t wakeup_flags;
  wait_queue_t *wait_queue;
  list_entry_t wait_link;
} wait_t;

#define le2wait(le, member) to_struct((le), wait_t, member)

void wait_init(wait_t *wait, struct proc_struct *proc);
void wait_queue_init(wait_queue_t *queue);
void wait_queue_add(wait_queue_t *queue, wait_t *wait);
void wait_queue_del(wait_queue_t *queue, wait_t *wait);

wait_t *wait_queue_next(wait_queue_t *queue, wait_t *wait);
wait_t *wait_queue_prev(wait_queue_t *queue, wait_t *wait);
wait_t *wait_queue_first(wait_queue_t *queue);
wait_t *wait_queue_last(wait_queue_t *queue);

bool wait_queue_empty(wait_queue_t *queue);
bool wait_in_queue(wait_t *wait);
void wakeup_wait(wait_queue_t *queue, wait_t *wait, uint32_t wakeup_flags, bool del);
void wakeup_first(wait_queue_t *queue, uint32_t wakeup_flags, bool del);
void wakeup_queue(wait_queue_t *queue, uint32_t wakeup_flags, bool del);

void wait_current_set(wait_queue_t *queue, wait_t *wait, uint32_t wait_state);

#define wait_current_del(queue, wait)                                       \
  do {                                                                    \
    if (wait_in_queue(wait)) {                                          \
      wait_queue_del(queue, wait);                                    \
    }                                                                   \
  } while (0)


void
wait_init(wait_t *wait, struct proc_struct *proc) {
  wait->proc = proc;
  wait->wakeup_flags = WT_INTERRUPTED;
  list_init(&(wait->wait_link));
}

void
wait_queue_init(wait_queue_t *queue) {
  list_init(&(queue->wait_head));
}

void
wait_queue_add(wait_queue_t *queue, wait_t *wait) {
  assert(list_empty(&(wait->wait_link)) && wait->proc != NULL);
  wait->wait_queue = queue;
  list_add_before(&(queue->wait_head), &(wait->wait_link));
}

void
wait_queue_del(wait_queue_t *queue, wait_t *wait) {
  assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
  list_del_init(&(wait->wait_link));
}

wait_t *
wait_queue_next(wait_queue_t *queue, wait_t *wait) {
  list_entry_t *le;
  assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
  le = list_next(&(wait->wait_link));
  if (le != &(queue->wait_head)) {
    return le2wait(le, wait_link);
  }
  return NULL;
}

wait_t *
wait_queue_prev(wait_queue_t *queue, wait_t *wait) {
  list_entry_t *le;
  assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
  le = list_prev(&(wait->wait_link));
  if (le != &(queue->wait_head)) {
    return le2wait(le, wait_link);
  }
  return NULL;
}

wait_t *
wait_queue_first(wait_queue_t *queue) {
  list_entry_t *le = list_next(&(queue->wait_head));
  if (le != &(queue->wait_head)) {
    return le2wait(le, wait_link);
  }
  return NULL;
}

wait_t *
wait_queue_last(wait_queue_t *queue) {
  list_entry_t *le = list_prev(&(queue->wait_head));
  if (le != &(queue->wait_head)) {
    return le2wait(le, wait_link);
  }
  return NULL;
}

bool
wait_queue_empty(wait_queue_t *queue) {
  return list_empty(&(queue->wait_head));
}

bool
wait_in_queue(wait_t *wait) {
  return !list_empty(&(wait->wait_link));
}

void sched_class_proc_tick(struct proc_struct *proc);

void wakeup_proc(struct proc_struct *proc);

void
wakeup_wait(wait_queue_t *queue, wait_t *wait, uint32_t wakeup_flags, bool del) {
  if (del) {
    wait_queue_del(queue, wait);
  }
  wait->wakeup_flags = wakeup_flags;
  wakeup_proc(wait->proc);
}

void
wakeup_first(wait_queue_t *queue, uint32_t wakeup_flags, bool del) {
  wait_t *wait;
  if ((wait = wait_queue_first(queue)) != NULL) {
    wakeup_wait(queue, wait, wakeup_flags, del);
  }
}

void
wakeup_queue(wait_queue_t *queue, uint32_t wakeup_flags, bool del) {
  wait_t *wait;
  if ((wait = wait_queue_first(queue)) != NULL) {
    if (del) {
      do {
	wakeup_wait(queue, wait, wakeup_flags, 1);
      } while ((wait = wait_queue_first(queue)) != NULL);
    }
    else {
      do {
	wakeup_wait(queue, wait, wakeup_flags, 0);
      } while ((wait = wait_queue_next(queue, wait)) != NULL);
    }
  }
}

void
wait_current_set(wait_queue_t *queue, wait_t *wait, uint32_t wait_state) {
  assert(current != NULL);
  wait_init(wait, current);
  current->state = PROC_SLEEPING;
  current->wait_state = wait_state;
  wait_queue_add(queue, wait);
}

#endif /* !__KERN_SYNC_WAIT_H__ */
