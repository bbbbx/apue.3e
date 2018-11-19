/**
 * 演示 vfork 函数
 */

#include "apue.h"

int    globvar = 6;     /* external 变量会在 initialized data 中 */
char   buf[] = "a write to stdout\n";

int main(void)
{
  int   var;      /* automatic 变量会在 stack 上 */
  pid_t pid;

  var = 88;
  
  printf("before vfork\n");    /* 我们还没有 flush stdout */

  if ((pid = vfork()) < 0) {
    err_sys("vfork error\n");
  } else if (pid == 0) {     /* 子进程 */
    globvar++;               /* 修改父进程的变量 */
    var++;
    _exit(0);                /* 终止子进程 */
  }

  /* parent continues here */
  printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar, var);

  return (0);
}
