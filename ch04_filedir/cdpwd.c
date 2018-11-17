#include "apue.h"

int main(void)
{
  char   *ptr;
  size_t size;

  if (chdir("/tmp") < 0)
    err_sys("chdir failed");

  ptr = path_alloc(&size);   /* 我们自己定义的函数，图 2.16 */

  if (getcwd(ptr, size) == NULL)
    err_sys("getcwd failed");

  printf("cwd = %s\n", ptr);

  exit(0);
}