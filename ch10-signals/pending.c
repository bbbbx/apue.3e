#include "apue.h"

static void   sig_quit(int);

int main(void)
{
  sigset_t newmask, oldmask, pendmask;

  /**
   * 在终端上按下 terminal quit key（通常是 Ctrl + \）
   * terminal driver  会产生 SIGQUIT signal
   */
  if (signal(SIGQUIT, sig_quit) == SIG_ERR)
    err_sys("can't catch SIGQUIT");
  
  /**
   * Block 掉 SIGQUIT 并保留目前的 signal mask
   */
  sigemptyset(&newmask);
  sigaddset(&newmask, SIGQUIT);   /* newmask 现在包含了 SIGQUIT */
  if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
    err_sys("SIG_BLOCK error");

  sleep(5);    /* 现在如果有 SIGQUIT signal 的话，则它还是 pending 的 */

  if (sigpending(&pendmask) < 0)
    err_sys("sigpending error");
  if (sigismember(&pendmask, SIGQUIT))
    printf("\nSIGQUIT pending\n");

  /**
   * 恢复 signal mask 为 unblocks SIGQUIT 的
   */
  if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
    err_sys("sigprocmask error");
  printf("SIGQUIT unblocked\n");

  sleep(5);     /* 现在如果有 SIGQUIT signal 的话，则进程会 terminate 并生成 core 文件  */
  exit(0);
}

static void sig_quit(int signo)
{
  printf("caught SIGQUIT\n");
  if (signal(SIGQUIT, SIG_DFL) == SIG_ERR)
    err_sys("can't reset SIGQUIT");
}

/**
 * 在 Mac OS X 10.14 的 /cores 目录下生成 core 文件的方法
 * 运行下面两条命令
 * $ sysctl kern.coredump=1     
 * $ ulimit -c unlimited
 * OK，搞定。
 * core 文件很大，小的都有 1G+
 */
