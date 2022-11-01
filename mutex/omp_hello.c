#include <omp.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  printf("hello\n");
#pragma omp parallel
  {
    int idx = omp_get_thread_num();
    int nth = omp_get_num_threads();
    for (int i = 0; i < 4; i++) {
      usleep(1000);
      printf("hi I am %d of %d\n", idx, nth);
    }
  }
  printf("bye\n");
  return 0;
}