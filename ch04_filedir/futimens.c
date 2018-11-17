#include "apue.h"
#include <fcntl.h>

int main(int argc, char const *argv[])
{
  int            i, fd;
  struct stat statbuf;
  struct timespec times[2];

  for (i = 1; i < argc; i++) {
    if (stat(argv[i], &statbuf) < 0) {
      err_ret("%s: stat error", argv[i]);
      continue;
    }
    if ((fd = open(argv[i], O_RDWR | O_TRUNC)) < 0) {
      err_ret("%s: open error", argv[i]);
      continue;
    }
    times[0].tv_sec = statbuf.st_atime;
    times[0].tv_nsec = statbuf.st_atimensec;
    times[1].tv_sec = statbuf.st_mtime;
    times[1].tv_nsec = statbuf.st_mtimensec;

    if (futimens(fd, times) < 0)
      err_ret("%s: futimens error", argv[i]);
    close(fd);
  }
  
  exit(0);
}
