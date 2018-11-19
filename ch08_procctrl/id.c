#include "apue.h"

int
main(void)
{
  pid_t pid = getpid();   
  pid_t ppid = getppid();  
  uid_t uid = getuid();
  uid_t euid = geteuid();
  gid_t gid = getgid();
  gid_t egid = getegid();

  printf("pid = %d\nppid = %d\nuid = %d\neuid = %d\ngid = %d\negid = %d\n", 
          pid, ppid, uid, euid, gid, egid);

  return (0);
}
