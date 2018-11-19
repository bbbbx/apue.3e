#include "apue.h"
#include <sys/wait.h>

int main(void)
{
  pid_t    pid;

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    if ((pid = fork()) < 0)
      err_sys("fork error");
    else if (pid > 0)
      exit(0);
    
    /**
     * 现在是第二个子进程，它的父进程变成了 init，因为它的父进程在上面调用
     * 了 exit()。现在我们继续执行第二个子进程的代码，如果第二个子进程终止了，
     * 则 init 会获得第二个子进程的终止状态
     */
    sleep(2);
    printf("second child, parent pid = %ld\n", (long)getppid());
    exit(0);
  }

  if (waitpid(pid, NULL, 0) != pid)   /* 等待第一个子进程终止 */
    err_sys("waitpid error");

  /**
   * 这里是父进程，最初的进程
   * 注意，它并不是第二个子进程的父进程
   * 
   *  父进程
   *  └── 第一个子进程
   *      └── 第二个子进程
   * 
   */
  exit(0); 
}
