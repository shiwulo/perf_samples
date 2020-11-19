#include <stdio.h>
#include <stdlib.h>

#define LEN 1024*1024*1024

char* memoryPool[8192];

int main () {
    for (int i=0; i< 8192; i++) {
        memoryPool[i] = (char *)malloc(8192);
    }
    while (1) {
        int i=random()%8192;
        int j=random()%8192;
        *(memoryPool[i]+j)=i+j;
    }
}