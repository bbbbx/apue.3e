#include "apue.h"
#include <limits.h>

int main(int argc, char const *argv[])
{
  long posix_version = sysconf(_SC_VERSION);
  long xsi_version = sysconf(_SC_XOPEN_VERSION);
  long open_max = sysconf(_SC_OPEN_MAX);
  long path_max = pathconf("/", _PC_PATH_MAX);
  printf("PATH_MAX = %lu, POSIX_VERSION = %lu, XOPEN_VERSION =  %lu, OPEN_MAX = %lu",
    path_max, posix_version, xsi_version, open_max);
  exit(0);
}
