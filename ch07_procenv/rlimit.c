#include "apue.h"
#include <sys/resource.h>

/* doit(RLIMIT_CORE) 将转为 pr_limits("RLIMIT_CORE", RLIMIT_CORE) */
#define doit(name)   pr_limits(#name, name)

static void   pr_limits(char *, int);

int
main(void)
{
  printf("%-14s%10s %10s\n\n", "资源", "rlim_cur", "rlim_max");

  #ifdef   RLIMIT_AS           /* 进程的 address space */
    doit(RLIMIT_AS);
  #endif

  doit(RLIMIT_CORE);           /* 一个 core file 的最大大小（bytes） */
  doit(RLIMIT_CPU);            /* 最大 CPU time 数，超出这个时间将向进程发送 SIGXCPU 信号 */
  doit(RLIMIT_DATA);           /* data segment 的最大大小，initialized data、uninitialized data 和 heap  的总大小 */
  doit(RLIMIT_FSIZE);          /* 可创建文件的大小 */
  
  #ifdef    RLIMIT_MEMLOCK
    doit(RLIM_MEMLOCK);
  #endif

  #ifdef    RLIMIT_MSGQUEUE
    doit(RLIMIT_MSGQUEUE);
  #endif

  #ifdef    RLIMIT_NICE
    doit(RLIMIT_NICE);
  #endif

  #ifdef    RLIMIT_NPROC
    doit(RLIMIT_NPROC);
  #endif

  doit(RLIMIT_NOFILE);            /* 每个进程能打开的最多文件数 */

  #ifdef    RLIMIT_RSS
    doit(RLIMIT_RSS);
  #endif

  #ifdef    RLIMIT_RSS
    doit(RLIMIT_RSS);
  #endif

  #ifdef    RLIMIT_SBSIZE
    doit(RLIMIT_SBSIZE);
  #endif

  #ifdef    RLIMIT_SIGPENDING
    doit(RLIMIT_SIGPENDING);
  #endif

  doit(RLIMIT_STACK);          /* stack 的最大 bytes 数 */

  #ifdef    RLIMIT_SWAP        /* 一个 user 可消耗的最大 swap space */
    doit(RLIMIT_SWAP);
  #endif

  #ifdef    RLIMIT_VMEME       /* RLIMIT_AS 的同义词，虚拟内存？ */
    doit(RLIMIT_VMEME);
  #endif

  exit(0);
}

static void
pr_limits(char *name, int resource)
{
  struct rlimit        limit;
  unsigned long long   lim;

  if (getrlimit(resource, &limit) < 0)
    err_sys("getrlimit error for %s", name);
  printf("%-14s", name);

  if (limit.rlim_cur == RLIM_INFINITY) {
    printf("(infinite) ");
  } else {
    lim = limit.rlim_cur;
    printf("%10lld ", lim);
  }

  if (limit.rlim_max == RLIM_INFINITY) {
    printf("(infinite) ");
  } else {
    lim = limit.rlim_max;
    printf("%10lld ", lim);
  }

  putchar((int)'\n');
}
