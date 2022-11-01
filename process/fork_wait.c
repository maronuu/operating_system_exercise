#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    return 123;  // my status
  } else {
    int ws;
    printf("parent: wait for child (pid=%d) to finish\n", pid);
    pid_t cid = waitpid(pid, &ws, 0);
    if (WIFEXITED(ws)) {
      printf("exited, status=%d\n", WEXITSTATUS(ws));
      fflush(stdout);
    } else if (WIFSIGNALED(ws)) {
      printf("killed by signal %d\n", WTERMSIG(ws));
      fflush(stdout);
    }
  }
  return 0;
}