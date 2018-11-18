#include "apue.h"
#include <errno.h>

void make_temp(char *template);

int main(void)
{
                            /* 其中 XXXXXX 为六个占位符 */
  char      good_template[] = "/tmp/dirXXXXXX";    /* 正确的方法 */
  char      *bad_template   = "/tmp/dirXXXXXX";    /* 错误的方法 */

  printf("正在尝试创建第一个临时文件...\n");
  make_temp(good_template);
  printf("正在尝试创建第二个临时文件...\n");
  make_temp(bad_template);
  exit(0);
}

void make_temp(char *template)
{
  int            fd;
  struct stat   sbuf;

  if ((fd = mkstemp(template)) < 0)
    err_sys("mkstemp error");

  printf("temp name = %s\n", template);
  close(fd);

  if (stat(template, &sbuf) < 0) {
    if (errno == ENOENT)
      printf("file doesn't exit\n"); 
    else
      err_sys("stat failed");
  } else {
    printf("file exists\n");
    unlink(template);
  }
}

/**
 * $ ./a.out
 * 正在尝试创建第一个临时文件...
 * temp name = /tmp/dirBCFXmof
 * file exists
 * 正在尝试创建第二个临时文件...
 * [1]    2117 bus error  ./a.out
 */

/**
 * 对于第一个 template，它的 name 是在 stack 上分配的，因为我们使用的是一个 array variable
 * 对于第一个 template，我们使用的是一个 pointer，在这种情况下，只有 pointer 自己留在 stack 上
 * compiler 会把 string 放在 read-only segment of the executable 上，
 * 所以会出现一个 segmentation fault
 */
