#include "../../libs/jlibc.h"

unsigned long *stack;
int child_tid;
int parent_tid;
uint32_t uaddr = 0;
void thread_function()
{
    _write_str(1, "Hello from the new thread!\n");
    int p = 1000;
    uaddr = 1;
    _futex(&uaddr, FUTEX_WAKE, 1, 0, 0, 0);
    _exit(0);
}
int main(int argc, char *argv[])
{
    unsigned long flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;

    stack = _mmap(stack, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    stack = (char *)stack + 4096;
    --stack;
    *stack = (unsigned long)thread_function;

    _clone(flags, stack, &parent_tid, &child_tid, 0);
    _write_str(1, "Hello from the main thread!\n");

    while (uaddr == 0)                          // loop guards against spurious/early return
        _futex(&uaddr, FUTEX_WAIT, 0, 0, 0, 0); // sleep WHILE it's still 0
    _write_str(1, "nmaybe\n");
    return 0;
}
