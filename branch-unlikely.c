#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

int main() {
    int rand=random()%2;
    double k=1088.7f,g=66783.66f;
    printf("pid = %d\n", getpid());
    for(int i=0; i<100000000000; i++){
        if (unlikely(random()%10 == 0)) {
            k++;
            g++;
            k=g/k;
            g=k/g;
        } else {
            g++;
            k++;
            k = g * k;
            g = k * g;
        }
    }
    return k+g;
}
