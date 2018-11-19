#include "apue.h"

static void charatatime(char *);

int main(void)
{
  pid_t    pid;

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    charatatime("output from child\n");
  } else {
    charatatime("output from parent\n");
  }
  exit(0);
}

static void charatatime(char *str)
{
  char  *ptr;
  int   c;

  setbuf(stdout, NULL);      /* 设置标准输出为不带缓冲的 */
  for(ptr = str; (c = *ptr++) != 0; )
    putc(c, stdout);
}
