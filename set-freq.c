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

#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB (30 << MAP_HUGE_SHIFT)

#define LENGTH (8192UL * 8192)

char *memoryPool;

void* keepmem(void* arg)
{
    int i = (intptr_t)arg;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(i, &set);
    assert(sched_setaffinity(getpid(), sizeof(set), &set) != -1);
    long long k;
    for (int j=0; j<8192; j++) {
        k += *(memoryPool+i*8192+j);
    }
}

int main()
{
    memoryPool = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB, 0, 0);
    if (memoryPool == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }
    printf("memoryPool = @%p\n", memoryPool);
    printf("tid is %d\n", gettid());

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    assert(sched_setaffinity(getpid(), sizeof(set), &set) != -1);

    pthread_t pid[16];

    for (int i=1; i<8; i++)
        assert(pthread_create(&pid[i], NULL, keepmem, (void *)(intptr_t)(i*2))!=0);

    printf("access shared array\n");
    while (1)
    {
        int i = random() % 8192;
        int j = random() % 8192;
        *(memoryPool+i*8192+j) = i + j;
    }
}