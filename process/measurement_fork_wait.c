#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

long cur_time() {
  struct timespec ts[1];
  clock_gettime(CLOCK_REALTIME, ts);
  return ts->tv_sec * 1000000000L + ts->tv_nsec;
}

int main(int argc, char** argv) {
  int n = (argc > 1 ? atoi(argv[1]) : 5);
  long t0 = cur_time();

  for (int i = 0; i < n; ++i) {
    pid_t pid = fork();
    if (pid == -1) {
      // fork failed
      err(1, "fork");
    } else if (pid == 0) {
      // child process
      char* const argv[] = {"./do_nothing", "", 0};
      execv(argv[0], argv);  // executes ./do_nothing
      err(1, "execv");
    } else {
      // parent process
      int ws;  // signal will be stored
      pid_t cid = waitpid(pid, &ws, 0);
      if (cid == -1) {
        // cid waitpid
        err(1, "waitpid");
      }
      // error handling
      if (WIFEXITED(ws)) {
        // printf("Exited (status=%d)\n", WEXITSTATUS(ws));
        fflush(stdout);
      } else if (WIFSIGNALED(ws)) {
        // printf("Killed by signal %d\n", WTERMSIG(ws));
        fflush(stdout);
      }
    }
  }

  long t1 = cur_time();
  long dt = t1 - t0;
  printf("%ld nsec to fork-exec-wait %d processes (%ld nsec/proc)\n", dt, n,
         dt / n);
  return 0;
}