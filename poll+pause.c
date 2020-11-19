#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>

char *memoryPool;

atomic_int seqNum=0;

long long successCount=0;

void* pollThread(void *arg)
{
    int myID = (intptr_t) arg;
    while(1) {
        while (myID != seqNum)
          asm("pause");
        successCount++;
        seqNum=(seqNum+1)%16;
    }
}

int main()
{
    pthread_t pid[16];

    for (int i=0; i<16; i++)
        pthread_create(&pid[i], NULL, pollThread, (void *)(intptr_t)i);
    sleep(120);
    printf("%lld \n", successCount);
}
