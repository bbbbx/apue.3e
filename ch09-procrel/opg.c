#include "apue.h"
#include <errno.h>

static void sig_hup(int signo)
{
  printf("SIGHUP received, pid = %ld\n", (long)getpid());
}

static void pr_ids(char *name)
{
  printf("%s: pid = %ld, ppid = %ld, pgrp = %ld, tpgrp = %ld\n",
              name, (long)getpid(), (long)getppid(), (long)getpgrp(), (long)tcgetpgrp(STDIN_FILENO));
  fflush(stdout);
}

int main(void)
{
  char      c; 
  pid_t     pid;

  pr_ids("parent");
  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid > 0) {   /* 父进程 */
    sleep(5);             /* 父进程睡眠，让子进程可以自己 stop 自己 */
  } else {                /* 子进程 */
    pr_ids("child");

    signal(SIGHUP, sig_hup);      /* establish 一个 hangup signal handler */
    kill(getpid(), SIGTSTP);      /* 子进程 stop 自己 */

    pr_ids("child");              /* 只有子进程继续执行的时候才会打印 */
    if (read(STDIN_FILENO, &c, 1) != 1)
      printf("read error %d on controlling TTY\n", errno);
  }

  exit(0);
}

/**
 * 没有在标准输入中输入 1
 * $ ./a.out
 * parent: pid = 4549, ppid = 512, pgrp = 4549, tpgrp = 4549
 * child: pid = 4550, ppid = 4549, pgrp = 4549, tpgrp = 4549
 * SIGHUP received, pid = 4550
 * child: pid = 4550, ppid = 1, pgrp = 4549, tpgrp = 4549
 * read error 5 on controlling TTY
 */

/** 
 * 在标准输入中输入 1
 * $ ./a.out
 * parent: pid = 4454, ppid = 512, pgrp = 4454, tpgrp = 4454
 * child: pid = 4455, ppid = 4454, pgrp = 4454, tpgrp = 4454
 * 1
 * SIGHUP received, pid = 4455
 * child: pid = 4455, ppid = 1, pgrp = 4454, tpgrp = 4454
 */

/**
 * 注意子进程 PPID 前后的变化
 * 子进程最后 PPID 变成了 1，即 init 进程
 */
