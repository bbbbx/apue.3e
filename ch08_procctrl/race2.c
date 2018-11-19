#include "apue.h"

static void charatatime(char *);

int main(void)
{
  pid_t    pid;

  TELL_WAIT();

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    // WAIT_PARENT();     /* 父进程先运行 */
    charatatime("output from child\n");
    TELL_PARENT(getppid());
  } else {
    WAIT_CHILD();         /* 子进程先运行 */
    charatatime("output from parent\n");
    // TELL_CHILD(pid);
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
