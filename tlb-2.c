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

long long count = 0;

void sigHandler(int signo)
{
    printf("%lld/sec\n", count);
    count = 0;
    exit(0);
}

int main () {
    signal(SIGALRM, sigHandler);
    alarm(30);
    char* memoryPool = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_HUGE_2MB, 0, 0);
    if (memoryPool == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    printf("memoryPool = @%p\n", memoryPool);
    printf("tid is %d\n", gettid());
    
    while (1)
    {
        count++;
        int i = random() % 8192;
        int j = random() % 8192;
        *(memoryPool+i*8192+j) = i + j;
    }
}
