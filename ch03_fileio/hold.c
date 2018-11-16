#include "apue.h"
#include "fcntl.h"

char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";

int main(int argc, char const *argv[])
{
  int fd;
  
  if ((fd = creat("file.hold", FILE_MODE)) < 0)
    err_sys("creat error");
  
  if ((write(fd, buf1, 10)) != 10)
    err_sys("buf1 write error");

  if ((lseek(fd, 16384, SEEK_SET)) == -1)
    err_sys("lseek error");
  /* 现在文件的 offset 是 16384 */

  if (write(fd, buf2, 10) != 10)
    err_sys("buf1 write error");  
  exit(0);
}
