#include "apue.h"
#include <sys/wait.h>

#define  DEF_PAGER     "/usr/bin/more"    /* default pager program */

int main(int argc, char *argv[])
{
  int     n;
  int     fd[2];
  pid_t   pid;
  char    *pager, *argv0;
  char    line[MAXLINE];
  FILE    *fp;

  if (argc != 2)
    err_quit("usage: %s <pathname>", argv[0]);

  if ((fp = fopen(argv[1], "r")) == NULL)
    err_sys("can't open %s", argv[1]);
  if (pipe(fd) < 0)
    err_sys("pipe error");

  if ((pid = fork()) < 0) {
    err_sys("fork error");
  } else if (pid > 0) {  /* 父进程 */
    close(fd[0]);   /* close read end */

    while (fgets(line, MAXLINE, fp) != NULL) {
      n = strlen(line);
      if (write(fd[1], line, n) != n)
        err_sys("write error to pipe");
    }
    if (ferror(fp))
      err_sys("fgets error");

    close(fd[1]);   /* close write end of pipe for reader */

    if (waitpid(pid, NULL, 0) < 0)
      err_sys("waitpid error");
    exit(0);
  } else {  /* 子进程 */
    close(fd[1]);  /* close write end */
    if (fd[0] != STDIN_FILENO) {
      if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
        err_sys("dup2 error to child");
      close(fd[0]);   /* don't need this after dup2 */
    }

    /* get arguments for execl() */
    if ((pager = getenv("PAGER")) == NULL)
      pager = DEF_PAGER;
    if ((argv0 = strrchr(pager, '/')) != NULL)
      argv0++;         /* 获取最右边的 slash 的后一位字符 */
    else
      argv0 = pager;   /* 在 pager 中没有 slash */
    
    if (execl(pager, argv0, (char *)0) < 0)
      err_sys("execl error for %s", pager);
  }
  
  exit(0);
}

/**
 * zsh 会将 PAGER 环境变量设为 "less"，
 * 应改为绝对路径 "/usr/bin/less"
 * $ export PAGER=/usr/bin/less
 */
