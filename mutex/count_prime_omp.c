
#include <err.h>
#include <omp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex;

int check_prime(long n) {
  for (long d = 2; d * d <= n; d++) {
    if (n % d == 0) return 0;
  }
  return n > 1;
}

void count_primes(long a, long b, long* s) {
#pragma omp for
  for (long n = a; n < b; n++) {
    if (check_prime(n)) {
      pthread_mutex_lock(&mutex);
      *s += 1;
      pthread_mutex_unlock(&mutex);
    }
  }
}

double cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_nsec * 1.0E-9 + ts->tv_sec;
}

int main(int argc, char** argv) {
  pthread_mutex_init(&mutex, NULL);
  long i = 1;
  long a = (argc > i ? atol(argv[i]) : 1000000);
  i++;
  long b = (argc > i ? atol(argv[i]) : 2000000);
  i++;
  long s = 0;
  double t0 = cur_time();
#pragma omp parallel
  { count_primes(a, b, &s); }
  double t1 = cur_time();
  printf("%ld primes in [%ld,%ld)\n", s, a, b);
  printf("%f sec\n", t1 - t0);
  pthread_mutex_destroy(&mutex);
  return 0;
}