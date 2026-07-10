#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

volatile long cnt = 0;
sem_t mutex;

void *TaskCode(void *vargp)
{
   long ntimes = *((long *)vargp);
   for (long i = 0; i < ntimes; i++)
   {
      sem_wait(&mutex);
      cnt++;
      sem_post(&mutex);
   }
   return NULL;
}

int main(int argc, char *argv[])
{
   sem_init(&mutex, 0, 1);
   pthread_t tid1, tid2, tid3;
   int rc;
   long ntimes = atoi(argv[1]);
   pthread_create(&tid1, NULL, TaskCode, &ntimes);
   pthread_create(&tid2, NULL, TaskCode, &ntimes);
   pthread_create(&tid3, NULL, TaskCode, &ntimes);
   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);
   pthread_join(tid3, NULL);
   if (cnt != 3 * ntimes)
   {
      printf("Error: cnt = %ld, expected %ld\n", cnt, 3 * ntimes);
      return 1;
   }
   printf("Success: cnt = %ld\n", cnt);
   return 0;
}