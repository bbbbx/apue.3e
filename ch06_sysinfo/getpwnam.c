#include <pwd.h>
#include <stddef.h>
#include <string.h>

/**
 * pwd.h 中的 getpwnae 的一个实现
 */
struct passwd *getpwnam(const char *name)
{
  struct passwd  *ptr;

  setpwent();
  while ((ptr = getpwent()) != NULL)
    if (strcmp(name, ptr->pw_name) == 0)
      break;      /* 找到了一个匹配项 */
  endpwent();
  return (ptr);   /* 如果没有找到了一个匹配项，则 ptr 为 NULL */
}
