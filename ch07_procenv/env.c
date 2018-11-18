#include <stdio.h>
#include <stdlib.h>

int main()
{
  putenv("MYNEWVAR=hello????");
  char *v = getenv("MYNEWVAR");

  printf("MYNEWVAR=%s\n", v);
}