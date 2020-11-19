#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

#define LENGTH (8192UL * 8192)

char memoryPool[8][1024*1024];;

void* keepmem(void* arg)
{
    int i = (intptr_t)arg;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(i, &set);
    //assert(sched_setaffinity(gettid(), sizeof(set), &set) != -1);
    long long k;

        for (int j=0; j<1024*1024; j++) {
            k += memoryPool[i][j];
        }
}


long long count=0;

void sigHandler(int signo) {
    printf("%lld/sec\n",count);
    count = 0;
    alarm(1);
}

int main()
{
    signal(SIGALRM, sigHandler);
    alarm(1);

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    assert(sched_setaffinity(gettid(), sizeof(set), &set) != -1);

    pthread_t pid[16];

    for (int i=1; i<8; i++) {
        pthread_create(&pid[i], NULL, keepmem, (void *)(intptr_t)(i*2));
        CPU_ZERO(&set);
        CPU_SET(i*2, &set);
        pthread_setaffinity_np(pid[i], sizeof(cpu_set_t), &set);
    }

    printf("memoryPool = @%p\n", memoryPool);
    printf("tid is %d\n", gettid());
    int res;
    while (1)
    {
        count++;
        int i = random() % 8;
        int j = random() % (1024*1024);
        res+=memoryPool[i][j];
    }
    return res;
}