#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

long ts2long(struct timespec ts)
{
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

int main(int argc, char** argv) {
    int fb;
    int cpuid;
    unsigned char *fb_mem;
    struct timespec t1, t2, t3, t4;

    sscanf(argv[1], "%d", &cpuid);
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuid, &mask); //SMT, hyper-threading

    fb = open("/dev/fb0", O_RDWR);
    assert(fb!=-1);
    fb_mem = mmap(NULL, 1024 * 1024 * 16, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i=0; i<10000; i++)
    memset(fb_mem, 0, 1024 * 1024 * 16); //这个命令应该只有在root可以执行
    clock_gettime(CLOCK_MONOTONIC, &t2);
    printf("worker: accessTime wo/ prefetch: %ld\n", ts2long(t2) - ts2long(t1));
}