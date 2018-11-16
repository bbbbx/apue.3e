#include "apue.h"
#include <fcntl.h>

int main(void)
{
  int fd1 = open("/Users/venus/workspace/apue/ch03_fileio", O_RDONLY);
  int fd2 = openat(fd1, "./apue.h", O_RDONLY);
  printf("%d, %d", fd1, fd2);
  exit(0);
}
