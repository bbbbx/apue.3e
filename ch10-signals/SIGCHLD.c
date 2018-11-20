#include "apue.h"
#include <sys/wait.h>

static void   sig_cld(int);

int main(void)
{
  pid_t    pid;

  if (signal(SIGCHLD, sig_cld) == SIG_ERR)
    perror("signal error");
  if ((pid = fork()) < 0) {
    perror("fork error");
  } else if (pid == 0) {   /* 子进程 */
    printf("child pid = %ld\n", (long)getpid());
    sleep(2);
    _exit(0);
  }

  pause();   /* 父进程 */
  exit(0);
}

static void sig_cld(int signo)
{
  pid_t   pid;
  int     status;

  printf("SIGCLD received, ");

  if (signal(SIGCHLD, sig_cld) == SIG_ERR)
    perror("signal error");

  if ((pid = wait(&status)) < 0)
    perror("wait error");

  printf("pid = %d\n", pid);
}
