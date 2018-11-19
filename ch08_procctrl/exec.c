#include "apue.h"
#include <sys/wait.h>

char *env_init[] = { "USER=unknown", "PATH=/tmp", NULL};

int main (void)
{
  pid_t    pid;

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {
    /**
     * 指定路径名，指定 environment。
     * 注意，我们将第一个参数 argv[0] 设置为了路径名的文件名部分，
     * 这只是惯例，可以将其设置为任何字符串。
     */
    if (execle("/Users/venus/workspace/apue/ch08_procctrl/echoall", "echoall", "myarg1", "MY ARG2", (char *)0, env_init) < 0)
      err_sys("execle error");
  }

  if (waitpid(pid, NULL, 0) < 0)
    err_sys("wait error");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid == 0) {    /* 明确的文件名，继承 environment */
    if (execlp("echoall", "echoall", "only 1 arg", (char *)0) < 0)
      err_sys("execlp error");
  }

  exit(0);
}
