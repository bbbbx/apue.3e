#include "apue.h"

int main(void)
{
  struct stat    statbuf;

  /* 开启 set-group-ID bit 和取消 group-execute */

  if (stat("foo", &statbuf) < 0)
    err_sys("stat error for foo");
  if (chmod("foo", (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0)
    err_sys("chmod error for foo");

  /* 将 mode 绝对地设置为 "rw-r--r--" */

  if (chmod("bar", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
    err_sys("chmod error for bar");    
    
  exit(0);
}
