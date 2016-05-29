#ifndef __KERN_SYNC_MONITOR_CONDVAR_H__
#define __KERN_SYNC_MOINTOR_CONDVAR_H__

#include <sem.h>
#include <io.h>
#include <kmalloc.h>
/* In [OS CONCEPT] 7.7 section, the accurate define and approximate implementation of MONITOR was introduced.
 * INTRODUCTION:
 *  Monitors were invented by C. A. R. Hoare and Per Brinch Hansen, and were first implemented in Brinch Hansen's
 *  Concurrent Pascal language. Generally, a monitor is a language construct and the compiler usually enforces mutual exclusion. Compare this with semaphores, which are usually an OS construct.
 * DEFNIE & CHARACTERISTIC:
 *  A monitor is a collection of procedures, variables, and data structures grouped together.
 *  Processes can call the monitor procedures but cannot access the internal data structures.
 *  Only one process at a time may be be active in a monitor.
 *  Condition variables allow for blocking and unblocking.
 *     cv.wait() blocks a process.
 *        The process is said to be waiting for (or waiting on) the condition variable cv.
 *     cv.signal() (also called cv.notify) unblocks a process waiting for the condition variable cv.
 *        When this occurs, we need to still require that only one process is active in the monitor. This can be done in several ways:
 *            on some systems the old process (the one executing the signal) leaves the monitor and the new one enters
 *            on some systems the signal must be the last statement executed inside the monitor.
 *            on some systems the old process will block until the monitor is available again.
 *            on some systems the new process (the one unblocked by the signal) will remain blocked until the monitor is available again.
 *   If a condition variable is signaled with nobody waiting, the signal is lost. Compare this with semaphores, in which a signal will allow a process that executes a wait in the future to no block.
 *   You should not think of a condition variable as a variable in the traditional sense.
 *     It does not have a value.
 *     Think of it as an object in the OOP sense.
 *     It has two methods, wait and signal that manipulate the calling process.
 * IMPLEMENTATION:
 *   monitor mt {
 *     ----------------variable------------------
 *     semaphore mutex;
 *     semaphore next;
 *     int next_count;
 *     condvar {int count, sempahore sem}  cv[N];
 *     other variables in mt;
 *     --------condvar wait/signal---------------
 *     cond_wait (cv) {
 *         cv.count ++;
 *         if(mt.next_count>0)
 *            signal(mt.next)
 *         else
 *            signal(mt.mutex);
 *         wait(cv.sem);
 *         cv.count --;
 *      }
 *
 *      cond_signal(cv) {
 *          if(cv.count>0) {
 *             mt.next_count ++;
 *             signal(cv.sem);
 *             wait(mt.next);
 *             mt.next_count--;
 *          }
 *       }
 *     --------routines in monitor---------------
 *     routineA_in_mt () {
 *        wait(mt.mutex);
 *        ...
 *        real body of routineA
 *        ...
 *        if(next_count>0)
 *            signal(mt.next);
 *        else
 *            signal(mt.mutex);
 *     }
 */

struct condvar;

struct monitor {
  semaphore_t mutex;        // the mutex lock for going into the routines in monitor, should be initialized to 1
  semaphore_t next;         // the next semaphore is used to down the signaling proc itself, and the other OR wakeuped waiting proc should wake up the sleeped signaling proc.
  int next_count;           // the number of of sleeped signaling proc
  struct condvar *cv;            // the condvars in monitor
};

typedef struct monitor monitor_t;

struct condvar {
  semaphore_t sem;          // the sem semaphore  is used to down the waiting proc, and the signaling proc should up the waiting proc
  int count;                // the number of waiters on condvar
  monitor_t * owner;        // the owner(monitor) of this condvar
};

typedef struct condvar condvar_t;

// Initialize variables in monitor.
void     monitor_init (monitor_t *cvp, size_t num_cv);
// Unlock one of threads waiting on the condition variable.
void     cond_signal (condvar_t *cvp);
// Suspend calling thread on a condition variable waiting for condition atomically unlock mutex in monitor,
// and suspends calling thread on conditional variable after waking up locks mutex.
void     cond_wait (condvar_t *cvp);


// Initialize monitor.
void
monitor_init (monitor_t * mtp, size_t num_cv) {
  int i;
  assert(num_cv>0);
  mtp->next_count = 0;
  mtp->cv = NULL;
  sem_init(&(mtp->mutex), 1);   //unlocked
  sem_init(&(mtp->next), 0);
  mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv);
  assert(mtp->cv!=NULL);
  for(i=0; i<num_cv; i++) {
    mtp->cv[i].count=0;
    sem_init(&(mtp->cv[i].sem),0);
    mtp->cv[i].owner=mtp;
  }
}

// Unlock one of threads waiting on the condition variable.
void
cond_signal (condvar_t *cvp) {
  //LAB7 EXERCISE1: 2013011343
  printf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
  /*
   *      cond_signal(cv) {
   *          if(cv.count>0) {
   *             mt.next_count ++;
   *             signal(cv.sem);
   *             wait(mt.next);
   *             mt.next_count--;
   *          }
   *       }
   */
  if(cvp->count>0) {
    cvp->owner->next_count++;
    up(&(cvp->sem));
    down(&(cvp->owner->next));
    cvp->owner->next_count--;
  }
  printf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks
// mutex and suspends calling thread on conditional variable after waking up locks mutex. Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
  //LAB7 EXERCISE1: 2013011343
  printf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
  /*
   *         cv.count ++;
   *         if(mt.next_count>0)
   *            signal(mt.next)
   *         else
   *            signal(mt.mutex);
   *         wait(cv.sem);
   *         cv.count --;
   */
  cvp->count++;
  if(cvp->owner->next_count > 0)
    up(&(cvp->owner->next));
  else
    up(&(cvp->owner->mutex));
  down(&(cvp->sem));
  cvp->count--;
  printf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

#endif /* !__KERN_SYNC_MONITOR_CONDVAR_H__ */
