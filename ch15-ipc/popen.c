#include "apue.h"
#include <sys/wait.h>

/* 如果有 shell variable PAGER，且非空，则使用该变量，否则使用字符串 more */
#define PAGER   "${PAGER:-more}"

int main(int argc, char *argv[])
{
  char line[MAXLINE];
  FILE *fpin, *fpout;

  if (argc != 2)
    err_quit("usage: %s <pathname>", argv[0]);

  if ((fpin = fopen(argv[1], "r")) == NULL)
    err_sys("can't open %s", argv[1]);

  if ((fpout = popen(PAGER, "w")) == NULL)
    err_sys("popen error");

  while (fgets(line, MAXLINE, fpin) != NULL) {
    if (fputs(line, fpout) == EOF)
      err_sys("fputs error to pipe");
  }

  if (ferror(fpin))
    err_sys("fgets error");
  if (pclose(fpout))
    err_sys("popen error");

  return 0;
}
