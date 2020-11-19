#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define LEN 1024*1024*1024

char* memoryPool[8192];

long long count;

void stopFun(int signo) {
    printf("%lld\n", count);
    exit(0);
}

int main () {
    signal(SIGALRM, stopFun);
    alarm(30);
    for (int i=0; i< 8192; i++) {
        memoryPool[i] = (char *)malloc(8192);
    }
    while (1) {
        count++;
        int i=random()%8192;
        int j=random()%8192;
        *(memoryPool[i]+j)=i+j;
    }
}
