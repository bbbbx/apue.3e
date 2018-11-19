#include "apue.h"
#include <sys/wait.h>
#include <signal.h>

int main(void)
{
  pid_t    pid;
  int      status;

  if ((pid = fork()) < 0)
    err_sys("fork error");
  else if (pid == 0)       /* 子进程 */
    exit(7);
  if (wait(&status) != pid)   /* 等待子进程 */
    err_sys("wait error");
  pr_exit(status);            /* 然后打印子进程的状态 */

  if ((pid = fork()) < 0)
    err_sys("fork error");
  else if (pid == 0)       /* 子进程 */
    abort();               /* 产生一个 SIGABRT */
  if (wait(&status) != pid)   /* 等待子进程 */
    err_sys("wait error");
  pr_exit(status);            /* 然后打印子进程的状态 */
  
  if ((pid = fork()) < 0)
    err_sys("fork error");
  else if (pid == 0)       /* 子进程 */
    status /= 0;               /* 除以 0 会产生一个 SIGFPE */
  if (wait(&status) != pid)   /* 等待子进程 */
    err_sys("wait error");
  pr_exit(status);            /* 然后打印子进程的状态 */

  exit(0);
}