#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();
  if (pid == -1) {
    err(1, "fork");
  } else if (pid == 0) {
    // child
    for (int i = 0; i < 5; ++i) {
      printf("This is child (%d): %d times\n", getpid(), i);
      fflush(stdout);
      usleep(100 * 1000);
    }
  } else {
    for (int i = 0; i < 5; ++i) {
      printf("This is parent (%d): %d times\n", getpid(), i);
      fflush(stdout);
      usleep(100 * 1000);
    }
  }
  return 0;
}