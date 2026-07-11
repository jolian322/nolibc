#include "jlibc.h"
#include "jmem.c"

void futex_wait(uint32_t *addr, uint32_t val)
{
    _futex(addr, FUTEX_WAIT, val, 0, 0, 0);
}

void futex_wake(uint32_t *addr, uint32_t val)
{
    _futex(addr, FUTEX_WAKE, val, 0, 0, 0);
}
typedef struct
{
    pid_t tid;
    void *mem_base;
    int joined;
} jthread_t;

jthread_t _jthreads[1024];
int _jthreadp = 0;

int jthread_create(jthread_t **handle, int attr, void (*thread_fn)(void *), void *arg)
{
    if (attr != 0)
    {
        // TODO
        return -1;
    }

    int idx = __atomic_fetch_add(&_jthreadp, 1, __ATOMIC_RELAXED);
    if (idx >= 1024)
    {
        __atomic_fetch_sub(&_jthreadp, 1, __ATOMIC_RELAXED);
        return -1;
    }
    jthread_t *jt = &_jthreads[idx];

    // init the stack
    unsigned long *stack = jstack();
    if (!stack)
    {
        return -1;
    }

    unsigned long *top = (unsigned long *)((char *)stack + STACK_SIZE);
    *(--top) = (unsigned long)_jthread_entry;
    *(--top) = (unsigned long)arg;
    *(--top) = (unsigned long)thread_fn;
    *(--top) = (unsigned long)_jthread_trampoline;

    jt->mem_base = (char *)stack - PAGE_SIZE;
    jt->tid = -1;
    unsigned long default_flags = CLONE_VM |
                                  CLONE_FS |
                                  CLONE_FILES |
                                  CLONE_SIGHAND |
                                  CLONE_THREAD |
                                  CLONE_CHILD_SETTID |
                                  CLONE_CHILD_CLEARTID;

    int retcode = _clone(default_flags,
                         (unsigned long)top,
                         NULL,     // parent_tid: unused for now
                         &jt->tid, // child_tid: kernel sets/clears this
                         0);       // tls: unused for now

    if (retcode < 0)
    {
        return retcode;
    }

    *handle = jt;
    return 0;
}

int jthread_join(jthread_t *handle)
{
    if (__atomic_exchange_n(&handle->joined, 1, __ATOMIC_ACQ_REL))
        return -1; // already joined
    pid_t cur;
    while ((cur = __atomic_load_n(&handle->tid, __ATOMIC_ACQUIRE)) != 0)
    {
        futex_wait((uint32_t *)&handle->tid, (uint32_t)cur);
    }
    _munmap(handle->mem_base, STACK_SIZE + (2 * PAGE_SIZE));
    return 0;
}

void _jthread_entry(void (*fn)(void *), void *arg)
{
    fn(arg);
    _exit(0);
}

__attribute__((naked)) void _jthread_trampoline(void)
{
    __asm__(
        "pop %rdi\n"
        "pop %rsi\n"
        "ret\n");
}