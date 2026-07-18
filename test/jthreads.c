#include "jthreads.h"

volatile long cnt = 0;
uint32_t mutex;


void TaskCode(void *vargp)
{
    long ntimes = *((long *)vargp);
    for (long i = 0; i < ntimes; i++)
    {
        jmutex_lock(&mutex);
        cnt++;
        jmutex_unlock(&mutex);
    }
}

int main(int argc, char *argv[])
{
    mutex = 1; // unlocked
    jthread_t* tid1, *tid2, *tid3;
    int rc;
    long ntimes = 1000;
    
    jthread_create(&tid1, 0, TaskCode, &ntimes);
    jthread_create(&tid2, 0, TaskCode, &ntimes);
    jthread_create(&tid3, 0, TaskCode, &ntimes);
    
    jthread_join(tid1);
    jthread_join(tid2);
    jthread_join(tid3);
    
    if (cnt != 3 * ntimes)
    {
        _write_str(1, "error\n");
        return 1;
    }
    
    _write_str(1, "succ\n");
    return 0;
}