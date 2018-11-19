#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

int mysystem(char *cmdstring)   /* 没有 signal handling 的 system */
{
  pid_t    pid;
  int      status;

  if (cmdstring == NULL)
    return (1);
  
  if ((pid = fork()) < 0) {
    status = -1;
  } else if (pid == 0) {    /* 子进程 */
    execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
    _exit(127);     /* execl error */
  } else {      /* 父进程 */
    while (waitpid(pid, &status, 0) < 0) {
      if (errno != EINTR) {
        status = -1;    /* error other than EINTR from waitpid() */
        break;
      }
    }
  }

  return (status);
}
