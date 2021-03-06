#include "apue.h"
#include <errno.h>
#include <stdarg.h>

static void err_doit(int, int, const char *, va_list);

/**
 * 出现一个和系统调用【有关】的【非致命性】错误
 * 打印一个 message 然后 return
 */
void err_ret(const char *fmt, ...)
{
  va_list        ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
}

/**
 * 出现一个和系统调用【有关】的【致命性】错误
 * 打印一个 message 然后 return
 */
void err_sys(const char *fmt, ...)
{
  va_list        ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  exit(1);
}

/**
 * 出现一个和系统调用【无关】的【非致命性】错误
 * 需要将 error code 作为一个参数显式地传进去
 * 打印一个 message 然后 return
 */
void err_cont(int error, const char *fmt, ...)
{
  va_list      ap;

  va_start(ap, fmt);
  err_doit(1, error, fmt, ap);
  va_end(ap);
}

/**
 * 出现一个和系统调用【无关】的【致命性】错误
 * 需要将 error code 作为一个参数显式地传进去
 * 打印一个 message 然后 terminate
 */
void err_exit(int error, const char *fmt, ...)
{
  va_list     ap;

  va_start(ap, fmt);
  err_doit(1, error, fmt, ap);
  va_end(ap);
  exit(1);
}

/**
 * 出现一个和系统调用【有关】的【致命性】错误
 * 打印一个 message、dump core，然后 terminate
 */
void err_dump(const char *fmt, ...)
{
  va_list        ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  abort();        /* dump core 和 terminate */
  exit(1);        /* 这行代码不会执行 */
}

/**
 * 出现一个和系统调用【无关】的【致命性】错误
 * 打印一个 message 然后 return
 */
void err_quit(const char *fmt, ...)
{
  va_list        ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
  exit(1);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
  char        buf[MAXLINE];

  vsnprintf(buf, MAXLINE-1, fmt, ap);
  if (errnoflag)
    snprintf(buf+strlen(buf), MAXLINE-strlen(buf)-1, ": %s", strerror(error));

  strcat(buf, "\n");
  fflush(stdout);
  fputs(buf, stderr);
  fflush(NULL);
}
