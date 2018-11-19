#include "apue.h"
#include <sys/wait.h>

int main(void)
{
  pid_t    pid;

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    if (execl("/Users/venus/workspace/apue/ch08_procctrl/testinterp",
              "testinterp", "myarg1", "MY ARG2", (char *)0) < 0)
      err_sys("execl error");
  }

  if (waitpid(pid, NULL, 0) < 0)   /* 父进程等待子进程终止 */
    err_sys("waitpid error");

  exit(0);
}