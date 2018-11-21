#include "apue.h"
#include <setjmp.h>
#include <time.h>

static void                    sig_usr1(int);
static void                    sig_alrm(int);
static sigjmp_buf              jmpbuf;
static volatile sig_atomic_t   canjump;

int main(void)
{
  if (signal(SIGUSR1, sig_usr1) < 0)
    err_sys("signal(SIGUSR1) error");
  if (signal(SIGALRM, sig_alrm) < 0)
    err_sys("signal(SIGALRM) error");
  
  pr_mask("starting main: ");     /* 图 10.14 */

  if (sigsetjmp(jmpbuf, 1)) {
    pr_mask("ending main: ");
    exit(0);
  }

  canjump = 1;  /* 现在可以调用 sigsetjmp() 了 */

  for (;;)
    pause();
}

static void sig_usr1(int signo)
{
  time_t       starttime;

  if (canjump == 0)
    return ;      /* 忽略不是我们自己产生的 SIGUSR1 signal */
  
  pr_mask("starting sig_usr1: ");

  alarm(3);
  starttime = time(NULL);
  for (;;)
    if (time(NULL) > starttime + 5)
      break;

  pr_mask("finishing sig_usr1: ");

  canjump = 0;
  siglongjmp(jmpbuf, 1);  /* 跳回到 main 中，don't return */
}

static void sig_alrm(int signo)
{
  pr_mask("in sig_alrm: ");
}

/**
 * 结果：
 * $ ./a.out &
 * [1] 5573
 * starting main:
 * 
 * $ kill -USR1 5573
 * starting sig_usr1:  SIGUSR1
 * 
 * $ in sig_alrm:  SIGUSR1
 * finishing sig_usr1:  SIGUSR1
 * ending main:
 * 
 * [1]  + 5573 done       ./a.out
 */
