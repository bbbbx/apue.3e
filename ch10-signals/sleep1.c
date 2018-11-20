/**
 * 简单又不完整的 sleep 的实现
 */

#include <signal.h>
#include <unistd.h>

static void sig_alrm(int signo)
{
  /* 什么也不做，只是为了 wake up 一个 pause */
}

unsigned int sleep1(unsigned int seconds)
{
  if (signal(SIGALRM, sig_alrm) == SIG_ERR)
    return (seconds);
  alarm(seconds);       /* 开始该 timer */
  pause();              /* 直到 caught 一个 signal 才 wake up */
  return (alarm(0));    /* 关掉 timer，返回 unslept time */
}
