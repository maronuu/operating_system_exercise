#include <assert.h>
#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int g = 0;

void *func(void *arg_) {
    long *arg = arg_;
    long n = arg[0];
    for (long i = 0; i < n; ++i) {
        ++g;
    }
    return 0;
}

int main(int argc, char **argv) {
    long n = (argc > 1 ? atol(argv[1]) : 1000000);
    long arg[1] = { n };

    const int nthreads = 2;
    pthread_t threads[nthreads];
    for (int i = 0; i < nthreads; ++i) {
        if (pthread_create(&threads[i], 0, func, arg)) {
            err(1, "pthread_create");
        }
    }
    for (int i = 0; i < nthreads; ++i) {
        void *ret = NULL;
        if (pthread_join(threads[i], &ret)) {
            err(1, "pthread_join");
        }
    }
    printf("g = %d\n", g);
    return 0;
}