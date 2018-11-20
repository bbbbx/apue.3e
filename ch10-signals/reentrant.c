#include "apue.h"
#include <pwd.h>

static void my_alarm(int signo)
{
  struct passwd  *venusptr;

  printf("in signal handler\n");
  if ((venusptr = getpwnam("venus")) == NULL)
    err_sys("getpwnam error");
  
  alarm(1);
}

int main(void)
{
  struct passwd  *ptr;

  signal(SIGALRM, my_alarm);
  alarm(1);     /* 1 秒后产生一个 SIGALRM 信号 */

  for (;;) {
    if ((ptr = getpwnam("venus")) == NULL)
      err_sys("getpwnam error");
    if (strcmp(ptr->pw_name, "venus") != 0)
      printf("return value corrupted!, pw_name = %s\n", ptr->pw_name);
  }

}
