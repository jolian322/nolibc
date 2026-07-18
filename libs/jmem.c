#include "jlibc.h"
#include "jmem.h"
#include "jds.c"

void* jmalloc(size_t bytes){
    void* mem = (void*)_mmap(NULL,PAGE_SIZE,PROT_READ | PROT_WRITE,0,-1,0);
    return mem;
}
int jfree(void* mem){
    int ret =  _munmap(mem, PAGE_SIZE);
    return ret;
}

void *jstack()
{
    char *mem = (char *)_mmap(
        NULL,
        STACK_SIZE + (2 * PAGE_SIZE),
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0);
    if (mem == NULL || mem == (char *)-1)
        return NULL;

    _mprotect(mem, PAGE_SIZE, PROT_NONE);
    _mprotect(mem + PAGE_SIZE + STACK_SIZE, PAGE_SIZE, PROT_NONE);
    return (void *)(mem + PAGE_SIZE);
}