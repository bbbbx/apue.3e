#include <stdio.h>

int main(void)
{
  unsigned int  number = 0x04030201;
  unsigned char *cp = (unsigned char *)&number;

  if (cp[0] == 0x01 && cp[3] == 0x04)
    printf("你的 CPU 是 little-endian 的\n");
  else if (cp[0] == 0x04 && cp[3] == 0x01)
    printf("你的 CPU 是 big-endian 的\n");
  else
    printf("鬼知道你的 CPU 是 little-endian 的还是 big-endian 的\n");

  return (0);
}