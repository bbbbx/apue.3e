#include <signal.h>
#include <errno.h>

#define SIGBAD(signo)((signo) <= 0 || (signo) >= NSIG)

int my_sigaddset(sigset_t *set, int signo)
{
  if (SIGBAD(signo)) {
    errno = EINVAL;
    return(-1);
  }
  *set |= 1 << (signo - 1);     /* 打开对应的 bit */
  return (0);
}

int my_sigdelset(sigset_t *set, int signo)
{
  if (SIGBAD(signo)) {
    errno = EINVAL;
    return(-1);
  }
  *set &= ~(1 << (signo - 1));  /* 关闭对应的 bit */
  return (0);
}

int sigmember(sigset_t *set, int signo)
{
  if (SIGBAD(signo)) {
    errno = EINVAL;
    return (-1);
  }
  return ((*set & (1 << (signo - 1)) != 0);
}
