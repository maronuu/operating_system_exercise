#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int compare_int(const void* a_, const void* b_) {
  const int* a = a_;
  const int* b = b_;
  return *a - *b;
}


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("Usage: %s <filename> <key>", argv[0]);
    return EXIT_FAILURE;
  }
  const char* filename = argv[1];
  const int key = atoi(argv[2]);

  // open file
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    err(1, "open");
  }
  // file size
  struct stat sb[1];
  if (fstat(fd, sb) == -1) {
    err(1, "fstat");
  }
  const long sz = (long)(sb->st_size);
  // linear search
  int buf[1024]; // 1024 elements
  int found = 0; // flg
  long rest = sz; // remaining
  const long offset = 1024 * sizeof(int); // 1024 elements
  long found_pos = 0; // found at
  while (rest > 0) {
    const long num_read = read(fd, buf, offset);
    if (num_read == 0) {
      break;
    }
    if (num_read == -1) {
      err(1, "read");
    }
    for (int i = 0; i < num_read / sizeof(buf[0]); i++) {
      if (buf[i] == key) {
        found = 1;
        found_pos = (sz - rest) / sizeof(int) + i;
      }
    }
    rest -= num_read;
  }
  if (found) {
    printf("%d found at %ld-th element", key, found_pos);
  } else {
    printf("%d not found", key);
  }

  if (close(fd) == -1) {
    err(1, "close");
  }
  return 0;
}