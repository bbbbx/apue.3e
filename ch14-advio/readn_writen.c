#include "apue.h"

ssize_t readn(int fd, void *ptr, size_t n)
{
  ssize_t nleft;
  ssize_t nread;

  nleft = 0;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (nleft == n)
        return (-1);   /* 出错，返回 -1 */
      else
        break;         /* 出错，返回已经读取到的数据 */
    } else if (nread == 0) {
      break;   /* EOF */
    }
    nleft -= nread;
    ptr += nread;
  }
  return (n - nleft);    /* return >= 0 */
}

ssize_t writen(int fd, const void *ptr, size_t n)
{
  size_t      nleft;
  ssize_t     nwritten;

  nleft = n;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) < 0) {
      if (nleft == n)
        return (-1);
      else
        break;
    } else if (nwritten == 0) {
      break;
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return (n - nleft);
}
