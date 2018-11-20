/**
 * sleep 的另一不完善的实现
 */

#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

static jmp_buf    env_alrm;

static void sig_alrm(int signo)
{
  longjmp(env_alrm, 1);
}

unsigned int sleep2(unsigned int seconds)
{
  if (signal(SIGALRM, sig_alrm) == SIG_ERR)
    return (seconds);
  if (setjmp(env_alrm) == 0) {
    alarm(seconds);       /* 开始该 timer */
    pause();              /* 直到 caught 一个 signal 才 wake up */
  }
  return (alarm(0));    /* 关掉 timer，返回 unslept time */
}
