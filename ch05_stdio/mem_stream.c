#include "apue.h"

#define BSZ 48

int main(void)
{
  FILE    *fp;
  char    buf[BSZ];

  memset(buf, 'a', BSZ-2);      /* 用 a 覆盖 buf*/
  buf[BSZ-2] = '\0';
  buf[BSZ-1] = 'X';
  
  if ((fp = fmemopen(buf, BSZ, "w+")) == NULL)    /* 用 fmemopen 在 buf 的开始处放一个 null byte*/
    err_sys("fmemopen error");
  
  printf("initial buffer contents: %s\n", buf);

  fprintf(fp, "hello, world");
  printf("before flush: %s\n", buf);
  fflush(fp);
  printf("after flush: %s\n", buf);
  printf("len of string in buf = %ld\n\n", (long)strlen(buf));


  memset(buf, 'b', BSZ-2);      /* 现在用 b 覆盖 buf*/
  buf[BSZ-2] = '\0';
  buf[BSZ-1] = 'X';
  fprintf(fp, "hello, world");
  fseek(fp, 0, SEEK_SET);
  printf("after fseek: %s\n", buf);
  printf("len of string in buf = %ld\n\n", (long)strlen(buf));

  
  memset(buf, 'c', BSZ-2);      /* 现在用 c 覆盖 buf*/
  buf[BSZ-2] = '\0';
  buf[BSZ-1] = 'X';
  fprintf(fp, "hello, world");
  fclose(fp);
  printf("after fclose: %s\n", buf);
  printf("len of string in buf = %ld\n", (long)strlen(buf));

  return (0);
}