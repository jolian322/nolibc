#include "jlibc.h"

void _futex_wait(uint32_t *addr, uint32_t val)
{
    _futex(addr, FUTEX_WAIT, val, 0, 0, 0);
}

void _futex_wake(uint32_t *addr, uint32_t val)
{
    _futex(addr, FUTEX_WAKE, val, 0, 0, 0);
}

pid_t thread_create(void (*thread_fn)(void))
{
    unsigned long *stack = _mmap(
        NULL,
        4096,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0);

    unsigned long *top = (unsigned long *)((char *)stack + 4096);

    *(--top) = (unsigned long)thread_fn;

    return _clone(
        CLONE_VM |
            CLONE_FS |
            CLONE_FILES |
            CLONE_SIGHAND |
            CLONE_THREAD,
        top,
        0,
        0,
        0);
}

void P(uint32_t *addr)
{
    while (1)
    {
        uint32_t cur = __atomic_load_n(addr, __ATOMIC_SEQ_CST);

        if (cur > 0)
        {
            uint32_t expected = cur;

            if (__atomic_compare_exchange_n(
                    addr,
                    &expected,
                    cur - 1,
                    0,
                    __ATOMIC_SEQ_CST,
                    __ATOMIC_SEQ_CST))
                return;

            continue;
        }

        _futex_wait(addr, 0);
    }
}

void V(uint32_t *addr)
{
    __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
    _futex_wake(addr, 1);
}

int ntimes = 100000;

volatile uint32_t cnt = 0;
uint32_t running_threads = 0;
void exit_thread(void)
{
    if (__atomic_sub_fetch(&running_threads, 1, __ATOMIC_SEQ_CST) == 0)
        _futex_wake(&running_threads, 1);

    _exit(0);
}
void thread_function(void)
{
    static uint32_t lock = 1;
    P(&lock);
    for (int i = 0; i < ntimes; i++)
    {
        
        cnt++;
    }
    V(&lock);

    exit_thread();
}

void wait_for_threads(void)
{
    while (1)
    {
        uint32_t cur = __atomic_load_n(&running_threads, __ATOMIC_SEQ_CST);

        if (cur == 0)
            return;

        _futex_wait(&running_threads, cur);
    }
}

int main(int argc, char *argv[])
{
    running_threads = 3;

    thread_create(thread_function);
    thread_create(thread_function);
    thread_create(thread_function);

    wait_for_threads();

    if (cnt != 3 * ntimes)
    {
        _write_str(1, "Error: cnt = ");
        _write_int(1, cnt);
        _write_str(1, ", expected ");
        _write_int(1, 3 * ntimes);
        _write_str(1, "\n");
        return 1;
    }
    _write_str(1, "Success: cnt = ");
    _write_int(1, cnt);
    _write_str(1, "\n");

    return 0;
}