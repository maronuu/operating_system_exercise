
#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_sec * 1000000000L + ts->tv_nsec;
}

void* do_nothing(void* arg) {
    pthread_t tid = pthread_self();
    printf("thread[%lu]: do_nothing called\n", tid);
    return arg;
}

int main(int argc, char** argv) {
  int n = (argc > 1 ? atoi(argv[1]) : 5);
  long t0 = cur_time();

  /* ここにプログラムを書く */
  pthread_t threads[n];
  void *ret;
  for (int i = 0; i < n; ++i) {
    if (pthread_create(&threads[i], NULL, do_nothing, NULL)) {
      err(1, "pthread_create");
    }
    if (pthread_join(threads[i], ret)) {
      err(1, "pthread_join");
    }
  }

  long t1 = cur_time();
  long dt = t1 - t0;
  printf("%ld nsec to pthrea_create-and-join %d threads (%ld nsec/thread)\n",
         dt, n, dt / n);
  return 0;
}