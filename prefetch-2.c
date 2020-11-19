//sudo perf stat -e mem_load_l3_hit_retired.xsnp_hit,mem_load_l3_hit_retired.xsnp_hitm,mem_load_l3_hit_retired.xsnp_miss,mem_load_l3_hit_retired.xsnp_none,offcore_response.other.l3_hit.any_snoop ./prefetch 40000

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <pthread.h> 
#include <stdatomic.h>
#include <sched.h>
#include <sys/sysinfo.h>

//for numa2
long ts2long(struct timespec ts) {
    return ts.tv_sec*1000000000 + ts.tv_nsec;
}


int localSize;
char* pool_with_prefetch; //with prefetch
char* pool_without_prefetch;

int numCore = -1;
atomic_int wait = 0;

int workerCPU;

void* worker(void* arg) {
    char *noprefetch_array = (char *)arg;
    //統計時間用
    struct timespec t1, t2, t3, t4;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(workerCPU, &mask); //SMT, hyper-threading
    sched_setaffinity(gettid(), sizeof(cpu_set_t), &mask);

    //printf("numCore = %d\n", numCore);

    //while (atomic_load_explicit(&wait, memory_order_relaxed) != numCore)
    //    ;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < localSize * numCore; i += 64)
    {
        __asm__(
            "movq 0(%0), %%rax\n\t"
            :
            : "r"(&noprefetch_array[0 + i])
            : "rax", "rbx", "rcx");
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    clock_gettime(CLOCK_MONOTONIC, &t3);
    for (int i = 0; i < localSize * numCore; i += 64)
    {
        __asm__(
            "movq 0(%0), %%rax\n\t"
            :
            : "r"(&pool_with_prefetch[0 + i])
            : "rax", "rbx", "rcx");
    }
    clock_gettime(CLOCK_MONOTONIC, &t4);

    printf("CPU#%-3d: worker: accessTime without prefetch: %ld\n", sched_getcpu(), ts2long(t2) - ts2long(t1));
    printf("CPU#%-3d: worker: accessTime with    prefetch: %ld\n", sched_getcpu(), ts2long(t4) - ts2long(t3));
}

void* prefetch(void* arg) {
    //char *array = pool_with_prefetch + ((intptr_t)arg) * numCore;
    char *array = (char *)arg;
    //統計時間用
    struct timespec t1, t2, t3, t4;
    
    //printf("實驗：『會』預先存取\n");
    //printf("大小為%ld Ｋ\n", sizeof(array)/1024);
    
    clock_gettime(CLOCK_MONOTONIC, &t1);
    //prefetch用的，讀取每個cache line的第一個 word (8B)
    for (int i = 0; i < localSize; i += 64)
    {
        __asm__ (
            //"movq %%rax, 0(%0)\n\t" 
            //"prefetchnta 0x00(%0)\n\t"    //效能最差
            "movq 0(%0), %%rax\n\t"                
            :                                
            : "r"(&array[0+i])
            : "rax");
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("CPU#%-3d: prefetch: %ld\n", sched_getcpu(), ts2long(t2) - ts2long(t1));
    //getchar();
    atomic_fetch_add_explicit(&wait, 1, memory_order_release);
    //printf("%d\n", wait);
}

int main(int argc, char** argv)
{
    pthread_t prefetcher_pid[16];
    pthread_t worker_pid;
    int numCPU = get_nprocs_conf();
    numCore = numCPU/2;
    sscanf(argv[1], "%d", &localSize);
    printf("在這個實驗中，prefetcher是一個接著一個執行，因此也可以看出prefetcher擷取資料的速度\n");
    printf("total size = %d\n", localSize * numCore);
    printf("# of CPU = %d\n", numCPU);
    pool_with_prefetch = aligned_alloc(64, localSize * numCore);
    pool_without_prefetch = aligned_alloc(64, localSize * numCore * numCore);

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < numCore; i++)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(i*2, &mask);    //SMT, hyper-threading
        //pthread_create(&prefetcher_pid[i], NULL, prefetch, (void *) (intptr_t)i);
        pthread_create(&prefetcher_pid[i], NULL, prefetch, (void *)&(pool_with_prefetch[localSize * i]));
        pthread_setaffinity_np(prefetcher_pid[i], sizeof(cpu_set_t), &mask);
        pthread_join(prefetcher_pid[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("total prefetch time = %.3f milliseconds\n", (ts2long(t2) - ts2long(t1)) / (1000000.0));

    for (int i = 0; i < numCore; i++) 
    {
        workerCPU = i;
        pthread_create(&worker_pid, NULL, worker, (void *)(pool_without_prefetch + (i * localSize * numCore)));
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(i, &mask); //SMT, hyper-threading
        //pthread_setaffinity_np(worker_pid, sizeof(cpu_set_t), &mask);

        pthread_join(worker_pid, NULL);
    }
}