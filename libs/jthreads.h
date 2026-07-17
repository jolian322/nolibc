#include "jlibc.h"
#ifndef JTHREADS
#define JTHREADS

typedef struct
{
    pid_t tid;      // thread id (kernel could zero this on threads exit and futex wake on it)
    void *mem_base; // membase (stack + the protection pages) // for freeing memory
    int joined;     // if joined already // to prevent double joinning
} jthread_t;        // thread handle structure

extern void futex_wait(uint32_t *addr, uint32_t val);                                          // futex sleep wrapper
extern void futex_wake(uint32_t *addr, uint32_t val);                                          // futex wake wrapper
extern int jthread_create(jthread_t **handle, int attr, void (*thread_fn)(void *), void *arg); // creates a thread returns 0 on sucess (writes a jthread_t handle) and if fail returns clone error code or -1 for others
extern int jthread_join(jthread_t *handle);                                                    // joins a thread returns 0 on success and -1 if already joined
extern void j_P(uint32_t *addr);                                                               // semaphore P operation
extern void j_V(uint32_t *addr);                                                               // semaphore V operation
extern void jmutex_lock(uint32_t *addr);                                                       // mutex lock operation
extern void jmutex_unlock(uint32_t *addr);                                                     // mutex unlock operation

#endif