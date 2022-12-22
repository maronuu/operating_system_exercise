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
  // mmap
  const long sz = (long)(sb->st_size);
  int* a = mmap(0, sz, PROT_READ, MAP_PRIVATE, fd, 0);
  if (a == MAP_FAILED) {
    err(1, "mmap");
  }
  // bsearch
  int* found = bsearch(&key, a, sz / sizeof(a[0]), sizeof(int), compare_int);
  if (found) {
    printf("%d found at %ld-th element\n", key, found - a);
  } else {
    printf("%d not found\n", key);
  }
  if (munmap(a, sz) == -1) {
    err(1, "munmap");
  }
  if (close(fd) == -1) {
    err(1, "close");
  }
  return 0;
}