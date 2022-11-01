#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *func(void *arg) {
  pthread_t tid = pthread_self();
  int slp_n = 5;
  for (int i = 0; i < slp_n; ++i) {
    printf("child[%lu]: (%d/%d)\n", tid, i, slp_n);
    fflush(stdout);
    usleep(100 * 1000);
  }
  return arg + 1;
}

int main() {
  pthread_t tid = pthread_self();
  pthread_t cid;
  if (pthread_create(&cid, NULL, func, NULL)) err(1, "pthread_create");
  int slp_n = 5;
  for (int i = 0; i < slp_n; ++i) {
    printf("parent[%lu]: (%d/%d)\n", tid, i, slp_n);
    fflush(stdout);
    usleep(100 * 1000);
  }

  void *ret;
  if (pthread_join(cid, &ret)) err(1, "pthread_join");
  printf("child thread returned %p\n", ret);
  return 0;
}