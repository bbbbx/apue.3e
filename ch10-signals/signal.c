#include "apue.h"

static void sig_usr(int);   /* 对所有的信号都用同一个 handler */

int main(void)
{
  if (signal(SIGUSR1, sig_usr) == SIG_ERR)
    err_sys("can't catch SIGUSR1");

  if (signal(SIGUSR2, sig_usr) == SIG_ERR)
    err_sys("can't catch SIGUSR2");

  for (;;)
    pause();
}

static void sig_usr(int signo)  /* 参数是 signal number */
{
  if (signo == SIGUSR1)
    printf("received SIGUSR1\n");
  else if (signo == SIGUSR2)
    printf("received SIGUSR2\n");
  else
    err_dump("received signal %d\n", signo);
}

/**
 * $ ./a.out  &
 * [1] 2733
 * $ kill -USR1 2733
 * received SIGUSR1
 * $ kill -USR2 2733
 * received SIGUSR2
 * $ kill 2733
 * [1]  + 2733 terminated  ./a.out
 */

/**
 * 注意：kill 命令默认发送的是 SIGTERM 信号
 */
