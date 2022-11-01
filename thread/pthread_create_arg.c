#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

typedef struct {
    long slp_usec;
    long slp_n;
    long id;
    pthread_t tid;
} thread_arg_t;

void *func(void *arg_) {
    thread_arg_t *arg = arg_;
    const long slp_usec = arg->slp_usec;
    const long slp_n = arg->slp_n;
    const long id = arg->id;
    const pthread_t tid = arg->tid;
    for (int i = 0; i < slp_n; ++i) {
        printf("child[%ld/%lu] (%d/%ld): sleep %ld usec\n", id, tid, i, slp_n, slp_usec);
        fflush(stdout);
        usleep(slp_usec);
    }
    return 0;
}

int main(int argc, char **argv) {
    int nthreads = (argc > 1 ? atoi(argv[1]): 3);
    thread_arg_t args[nthreads];

    for (int i = 0; i < nthreads; ++i) {
        args[i].slp_n = i + 2;
        args[i].slp_usec = 1000 * 1000 / args[i].slp_n;
        args[i].id = i;
        if (pthread_create(&args[i].tid, 0, func, &args[i])) {
            err(1, "pthread_create");
        }
    }

    for (int i = 0; i < nthreads; ++i) {
        void *ret;
        if (pthread_join(args[i].tid, &ret)) {
            err(1, "pthread_join");
        }
        assert(ret == 0);
        printf("child thread %d returned (%p)\n", i, ret);
    }
    return 0;
}