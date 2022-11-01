#include <assert.h>
#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int g = 0;
pthread_mutex_t mutex;

int main(int argc, char** argv) {
  long n = (argc > 1 ? atol(argv[1]) : 1000000);
  g = 0;
  pthread_mutex_init(&mutex, NULL);
#pragma omp parallel
  {
#pragma omp for
    for (long i = 0; i < n; i++) {
      pthread_mutex_lock(&mutex);
      g++;
      pthread_mutex_unlock(&mutex);
    }
  }
  printf("g = %d\n", g);
  pthread_mutex_destroy(&mutex);
  return 0;
}
