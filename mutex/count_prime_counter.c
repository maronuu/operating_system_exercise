
#include <err.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int check_prime(long n) {
  for (long d = 2; d * d <= n; d++) {
    if (n % d == 0) return 0;
  }
  return n > 1;
}

typedef struct {
  long n;
  pthread_mutex_t mutex;
} counter_t;

void counter_init(counter_t* c) {
  c->n = 0;
  pthread_mutex_init(&c->mutex, NULL);
  return;
}

long counter_inc(counter_t* c) {
  long ret;
  pthread_mutex_lock(&c->mutex);
  ret = c->n;
  (c->n)++;
  pthread_mutex_unlock(&c->mutex);
  return ret;
}

long counter_get(counter_t* c) {
  long ret;
  pthread_mutex_lock(&c->mutex);
  ret = c->n;
  pthread_mutex_unlock(&c->mutex);
  return ret;
}

void count_primes(long a, long b, counter_t* c) {
#pragma omp for
  for (long n = a; n < b; n++) {
    if (check_prime(n)) {
      counter_inc(c);
    }
  }
}

double cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_nsec * 1.0E-9 + ts->tv_sec;
}

int main(int argc, char** argv) {
  long i = 1;
  long a = (argc > i ? atol(argv[i]) : 1000000);
  i++;
  long b = (argc > i ? atol(argv[i]) : 2000000);
  i++;
  counter_t c[1];
  counter_init(c);
  double t0 = cur_time();
#pragma omp parallel
  {
    /* 起動時に環境変数OMP_NUM_THREADS=xxx で指定した
       個数のスレッドが作られ, 各々が以下の文 { ... }
       を実行する.
       関数内のpragma omp for 下のfor文をそれらのスレッドが
       分割して実行する */
    count_primes(a, b, c);
  }
  double t1 = cur_time();
  printf("%ld primes in [%ld,%ld)\n", counter_get(c), a, b);
  printf("%f sec\n", t1 - t0);
  return 0;
}