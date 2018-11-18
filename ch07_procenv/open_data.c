#include <stdio.h>

FILE *
open_date(void)
{
  FILE    *fp;
  char    datebuf[BUFSIZ];   /* setvbuf makes this the stdio buffer */

  if ((fp = fopen("datafile", "r")) == NULL)
    return (NULL);
  if (setvbuf(fp, datebuf, _IOLBF, BUFSIZ) != 0)
    return (NULL);
  return (fp);      /* error */
}
