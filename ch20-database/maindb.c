#include "apue.h"
#include "apue_db.h"
#include <fcntl.h>

int main(void)
{
  DBHANDLE   db;
  char       *val;

  if ((db = db_open("maindb", O_RDWR | O_CREAT | O_TRUNC, FILE_MODE)) == NULL)
    err_sys("db_open error");

  if (db_store(db, "001", "Venus", DB_INSERT) != 0)
    err_quit("db_store error for 001");
  if (db_store(db, "002", "Shell", DB_INSERT) != 0)
    err_quit("db_store error for 002");

  if ((val = db_fetch(db, "002")) < 0)
    err_quit("db_fetch error for 002");
  printf("before delete: 002 is %s\n", val);

  if (db_delete(db, "002") < 0)
    err_quit("db_delete error for 002");
  printf("after  delete: 002 is %s\n", val);

  db_close(db);
  exit(0);
}
