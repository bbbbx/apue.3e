/**
 * 演示 fork 函数
 * 子进程改变自己的变量并不影响父进程中的变量
 */

#include "apue.h"

int    globvar = 6;     /* external 变量会在 initialized data 中 */
char   buf[] = "a write to stdout\n";

int main(void)
{
  int   var;      /* automatic 变量会在 stack 中 */
  pid_t pid;

  var = 88;
  /* write 函数是不带缓冲的，但 standard I/O library 是带缓冲的 */
  if (write(STDOUT_FILENO, buf, sizeof(buf)-1) != sizeof(buf)-1)
    err_sys("write error");
  
  printf("before fork\n");    /* 我们还没有 flush stdout */

  if ((pid = fork()) < 0) {
    err_sys("fork error\n");
  } else if (pid == 0) {     /* 子进程 */
    globvar++;
    var++;
  } else {                  /* 父进程 */
    sleep(2);
  }

  printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);

  return (0);
}
