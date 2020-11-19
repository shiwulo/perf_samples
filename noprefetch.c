#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024 * 1024 * 128
int globalData[SIZE];
char randArray[32];

int main(int argc, char** argv) {
    //最多三個引數
    //address, locality(0..3, default=0(little or no temporal locality)), rw (0=read, 1=write, default=0)
    //__builtin_prefetch(globalData);

    int prefetchIdx = 0;
    for (int k = 0; k < 10; k++)
        for (int i = 0; i < SIZE; i++)
        {
            int curIdx = prefetchIdx;
            int prefetchIdx = (prefetchIdx + 17) % SIZE;
            __builtin_prefetch(globalData + prefetchIdx);
            globalData[(curIdx) % SIZE] = globalData[(curIdx) % SIZE] * 3.2f + 1;
        }
}