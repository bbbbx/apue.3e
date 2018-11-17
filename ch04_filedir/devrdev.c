#include "apue.h"

#ifdef SOLARIS
  #include <sys/mkdev.h>
#endif

#ifdef __APPLE__
  #ifndef major
  #define	major(x)	((int)(((unsigned int)(x) >> 24) & 0xff))
  #endif

  #ifndef minor
  #define	minor(x)	((int)((x) & 0xffffff))
  #endif
#endif

#ifdef __linux__
  #ifndef major
  #define	major(x)	((int)(((unsigned int)(x) >> 20) & 0xfff))
  #endif

  #ifndef minor
  #define	minor(x)	((int)((x) & 0xfffff))
  #endif
#endif

int main(int argc, char *argv[])
{
  int         i;
  struct stat buf;
  for (i = 1; i < argc; i++) {
    printf("%s: ", argv[i]);
    if (stat(argv[i], &buf) < 0) {
      err_sys("stat error for %s", argv[i]);
      continue;
    }

    printf("dev = %d/%d", major(buf.st_dev), minor(buf.st_dev));
    if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode)) {
      printf(", (%s) rdev = %d/%d", (S_ISCHR(buf.st_mode)) ? "character" : "block", major(buf.st_rdev), minor(buf.st_rdev));
    }
    printf("\n");
  }
  exit(0);
}
