#include "apue.h"
#include <errno.h>

int main(int argc, char const *argv[])
{
  fprintf(stdout, "EACCES: %s\n", strerror(EACCES));
  errno = ENOENT;
  perror(argv[0]);

  exit(0);
}
