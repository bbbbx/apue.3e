#include "apue.h"

int main(int argc, char *argv[])
{
  int          i;
  char         **ptr;
  extern char  **environ;

  for (i = 0; i < argc; i++)   /* echo 所有的 command-line args */
    printf("argv[%d]: %s\n", i, argv[i]);

  for (ptr = environ; *ptr != 0; ptr++)   /* echo 所有的 env strings */
    printf("%s\n", *ptr);

  return 0;
}
