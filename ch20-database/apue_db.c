#include "apue.h"
#include "apue_db.h"
#include <fcntl.h>     /* open & db_open flags */
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>   /* struct iovec */

/**
 * Internal index file constants.
 * These are used to construct record in
 * the index file and data file.
 */
#define IDXLEN_SZ       4
#define SEP           ':'
#define SPACE         ' '
#define NEWLINE       '\n'

/**
 * 接下来的定义是给在 index file 中的
 * hash chains 和 free list chain 用的。
 */
#define PTR_SZ          7
#define PTR_MAX    999999
#define NHASH_DEF     137     /* default hash table size */
#define FREE_OFF        0     /* free list offset in index file */
#define HASH_OFF   PTR_SZ     /* hash table offset in index file */

typedef unsigned long DBHASH;   /* hash values */
typedef unsigned long COUNT;    /* unsigned counter */

typedef struct {
  int       idxfd;      /* index file 的 file descriptor */
  int       datfd;      /* data file 的 file descriptor */
  char      *idxbuf;    /* index file 的 malloc'ed buffer */
  char      *datbuf;    /* data file 的 malloc'ed buffer */
  char      *name;      /* name db was opend under */
  off_t     idxoff;     /* index file 中的 index record 的 offset */
                        /* key 位于 (idxoff + PTR_SZ + IDXLEN_SZ) */
  size_t    idxlen;     /* length of index record */
                        /* excludes IDXLEN_SZ bytes at front of record */
                        /* includes newline at end of index record */
  off_t     datoff;     /* offset in data file of data record */
  size_t    datlen;     /* length of data record */
                        /* includes newline at end */
  off_t     ptrval;     /* contents of chain ptr in index record */
  off_t     ptroff;     /* chain ptr offset pointing to this index record */
  off_t     chainoff;   /* offset of hash chain for this index record */
  off_t     hashoff;    /* offset in index file of hash table */
  DBHASH    nhash;      /* current hash table size */
  COUNT     cnt_delok;
  COUNT     cnt_delerr;
  COUNT     cnt_fetchok;
  COUNT     cnt_fetcherr;
  COUNT     cnt_nextrec;    /* nextrec */
  COUNT     cnt_stor1;      /* store: DB_INSERT, no empty, appended */
  COUNT     cnt_stor2;      /* store: DB_INSERT, found empty, reused */
  COUNT     cnt_stor3;      /* store: DB_REPLACE, diff len, appended */
  COUNT     cnt_stor4;      /* store: DB_REPLACE, same len, overwrote */
  COUNT     cnt_storerr;    /* store error */
} DB;

/**
 * 内部的函数
 */
static DB     *_db_alloc(int);
static void   _db_dodelete(DB *);
static int    _db_find_and_lock(DB *, const char *, int);
static int    _db_findfree(DB *, int, int);
static void   _db_free(DB *);
static DBHASH _db_hash(DB *, const char *);
static char   *_db_readdat(DB *);
static off_t  _db_readidx(DB *, off_t);
static off_t  _db_readptr(DB *, off_t);
static void   _db_writedat(DB *, const char *, off_t, int);
static void   _db_writeidx(DB *, const char *, off_t, int, off_t);
static void   _db_writeptr(DB *, off_t, off_t);

/**
 * Open or create a database. Same arguments as open(2)
 */
DBHANDLE db_open(const char *pathname, int oflag, ...)
{
  DB          *db;
  int         len, mode;
  size_t      i;
  char        asciiptr[PTR_SZ + 1],
              hash[(NHASH_DEF + 1) * PTR_SZ + 2];
              /* +2 for newline and null */
  struct stat statbuff;

  len = strlen(pathname);
  if ((db = _db_alloc(len)) == NULL)
    err_dump("db_open: _db_alloc error for DB");
  db->nhash = NHASH_DEF;    /* hash table size */
  db->hashoff = HASH_OFF;   /* hash table 在 index file 中的 offset */
  strcpy(db->name, pathname);
  strcat(db->name, ".idx");

  if (oflag & O_CREAT) {
    va_list  ap;

    va_start(ap, oflag);
    mode = va_arg(ap, int);
    va_end(ap);

    /**
     * Open index file and data file.
     */
    db->idxfd = open(db->name, oflag, mode);
    strcpy(db->name + len, ".dat");
    db->datfd = open(db->name, oflag, mode);
  } else {
    /**
     * Open index file and data file.
     */
    db->idxfd = open(db->name, oflag);
    strcpy(db->name + len, ".dat");
    db->datfd = open(db->name, oflag);
  }

  if (db->idxfd < 0 || db->datfd < 0) {
    _db_free(db);
    return (NULL);
  }

  /**
   * 如果创建了 database，则我们必须 initialize 它。
   * 先 lock 住整个文件，这样我们才能 stat 它，
   * 查看它的 size 并 initialize 它。
   */
  if ((oflag & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) {
    if (writew_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
      err_dump("dopen: write_lock error");

    if (fstat(db->idxfd, &statbuff) < 0)
      err_sys("db_open: fstat error");

    if (statbuff.st_size == 0) {
      /**
       * 如果 index file 的 size 为 0,
       * 则用 0 来初始化 free ptr 和 chain ptr。
       */
      sprintf(asciiptr, "%*d", PTR_SZ, 0);
      hash[0] = 0;
      for (i = 0; i < NHASH_DEF + 1; i++)
        strcat(hash, asciiptr);
      strcat(hash, "\n");
      i = strlen(hash);
      if (write(db->idxfd, hash, i) != i)
        err_dump("db_open: index file init write error");
    }
    if (un_lock(db->idxfd, 0, SEEK_SET, 0) < 0)
      err_dump("db_open: un_lock error");
  }
  db_rewind(db);
  return(db);
}

static DB *_db_alloc(int namelen)
{
  DB      *db;

  if ((db = calloc(1, sizeof(DB))) == NULL)
    err_dump("_db_alloc: calloc error for DB");
  db->idxfd = db->datfd = -1;

  if ((db->name = malloc(namelen + 5)) == NULL)
    err_dump("_db_alloc: malloc error for name");
  if ((db->idxbuf = malloc(IDXLEN_MAX + 2)) == NULL)
    err_dump("_db_alloc: malloc error for index buffer");
  if ((db->datbuf = malloc(DATLEN_MAX + 2)) == NULL)
    err_dump("_db_alloc: malloc error for data buffer");
  return (db);
}

/**
 * Relinquish access to the database
 */
void db_close(DBHANDLE h)
{
  _db_free((DB *)h);   /* close fds, free buffers & struct */
}

static void _db_free(DB *db)
{
  if (db->idxfd >= 0)
    close(db->idxfd);
  if (db->datfd >= 0)
    close(db->datfd);

  if (db->idxbuf != NULL)
    free(db->idxbuf);
  if (db->datbuf != NULL)
    free(db->datbuf);
  if (db->name != NULL)
    free(db->name);
  free(db);
}

char *db_fetch(DBHANDLE h, const char *key)
{
  DB     *db = h;
  char   *ptr;

  if (_db_find_and_lock(db, key, 0) < 0) {
    ptr = NULL;          /* error, record not found */
    db->cnt_fetcherr++;
  } else {
    ptr = _db_readdat(db);  /* return pointer to data */
    db->cnt_fetchok++;
  }

  if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
    err_dump("db_fetch: un_lock error");
  return (ptr);
}

/**
 * 找到指定的 record。
 * 由 db_delete、db_fetch 和 db_store 调用。
 * 返回带有 locked 的 hash chain。
 */
static int _db_find_and_lock(DB *db, const char *key, int writelock)
{
  off_t offset, nextoffset;

  db->chainoff = (_db_hash(db, key) * PTR_SZ) + db->hashoff;
  db->ptroff = db->chainoff;

  if (writelock) {
    if (writew_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
      err_dump("_db_find_and_lock: writew_lock error");
  } else {
    if (readw_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
      err_dump("_db_find_and_lock: readw_lock error");
  }
  
  /**
   * Get the offset in the index file of first record
   * on the hash chain (can be 0)
   */
  offset = _db_readptr(db, db->ptroff);

  while (offset != 0) {
    nextoffset = _db_readidx(db, offset);
    if (strcmp(db->idxbuf, key) == 0)
      break;    /* 找到了一个匹配项 */
    db->ptroff = offset;      /* offset of this (unequal) record */
    offset = nextoffset;      /* next one to compare */
  }

  /**
   * offset == 0 on error (record not found)
   */
  return (offset == 0 ? -1 : 0);
}

static DBHASH _db_hash(DB *db, const char *key)
{
  DBHASH     hval = 0;
  char       c;
  int        i;

  for (i = 1; (c = *key++) != 0; i++)
    hval += c * i;            /* ascii char times its 1-based index */
  return (hval % db->nhash);
}

static off_t _db_readptr(DB *db, off_t offset)
{
  char asciiptr[PTR_SZ + 1];
  if (lseek(db->idxfd, offset, SEEK_SET) == -1)
    err_dump("_db_readptr: lseek error to ptr field");
  if (read(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ)
    err_dump("_db_readptr: read error of ptr field");
  asciiptr[PTR_SZ] = 0;
  return (atol(asciiptr));
}

static off_t _db_readidx(DB *db, off_t offset)
{
  ssize_t            i;
  char               *ptr1, *ptr2;
  char               asciiptr[PTR_SZ + 1], asciilen[IDXLEN_SZ + 1];
  struct iovec       iov[2];

  if ((db->idxoff = lseek(db->idxfd, offset, 
    offset == 0 ? SEEK_CUR : SEEK_SET)) == -1)
    err_dump("_db_readidx: lseek error");
  
  iov[0].iov_base = asciiptr;
  iov[0].iov_len = PTR_SZ;
  iov[1].iov_base = asciilen;
  iov[1].iov_len = IDXLEN_SZ;

  if ((i = readv(db->idxfd, &iov[0], 2)) != PTR_SZ + IDXLEN_SZ) {
    if (i == 0 && offset == 0)
      return (-1);        /* EOF for db_nextrec */
    err_dump("_db_readidx: readv error of index record");
  }

  asciiptr[PTR_SZ] = 0;
  db->ptrval = atol(asciiptr);

  asciilen[IDXLEN_SZ] = 0;
  if ((db->idxlen = atoi(asciilen)) < IDXLEN_MIN ||
    db->idxlen > IDXLEN_MAX)
    err_dump("_db_readidx: invalid length");
  
  if ((i = read(db->idxfd, db->idxbuf, db->idxlen)) != db->idxlen)
    err_dump("_db_readidx: read error of index record");
  if (db->idxbuf[db->idxlen - 1] != NEWLINE)
    err_dump("_db_readidx: missing newline");

  db->idxbuf[db->idxlen - 1] = 0;

  if ((ptr1 = strchr(db->idxbuf, SEP)) == NULL)
    err_dump("_db_readidx: missing first separator");
  *ptr1++ = 0;

  if ((ptr2 = strchr(ptr1, SEP)) == NULL)
    err_dump("_db_readidx: missing second separator");
  *ptr2++ = 0;

  if (strchr(ptr2, SEP) != NULL)
    err_dump("_db_readidx: too many separators");

  if ((db->datoff = atol(ptr1)) < 0)
    err_dump("_db_readidx: starting offset < 0");
  if ((db->datlen = atol(ptr2)) <= 0 || db->datlen >DATLEN_MAX)
    err_dump("_db_readidx: invalid length");
  return (db->ptrval);
}

static char *_db_readdat(DB *db)
{
  if (lseek(db->datfd, db->datoff, SEEK_SET) == -1)
    err_dump("_db_readdat: lseek error");
  if (read(db->datfd, db->datbuf, db->datlen) != db->datlen)
    err_dump("_db_readdat: read error");
  if (db->datbuf[db->datlen - 1] != NEWLINE)
    err_dump("_db_readdat: missing newline");
  db->datbuf[db->datlen - 1] = 0;
  return (db->datbuf);
}

int db_delete(DBHANDLE h, const char *key)
{
  DB     *db = h;
  int    rc = 0;

  if (_db_find_and_lock(db, key, 1) == 0) {
    _db_dodelete(db);
    db->cnt_delok++;
  } else {
    rc = -1;
    db->cnt_delerr++;
  }

  if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
    err_dump("db_delete: un_lock error");
  return (rc);
}

static void _db_dodelete(DB *db)
{
  int     i;
  char    *ptr;
  off_t   freeptr, saveptr;

  for (ptr = db->datbuf, i = 0; i < db->datlen - 1; i++)
    *ptr++ = SPACE;
  
  *ptr = 0;
  ptr = db->idxbuf;
  while (*ptr)
    *ptr++ = SPACE;

  if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("_db_dodelete: writew_lock error");
  
  _db_writedat(db, db->datbuf, db->datoff, SEEK_SET);

  freeptr = _db_readptr(db, FREE_OFF);

  saveptr = db->ptrval;

  _db_writeidx(db, db->idxbuf, db->idxoff, SEEK_SET, freeptr);

  _db_writeptr(db, FREE_OFF, db->idxoff);

  _db_writeptr(db, db->ptroff, saveptr);
  if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("_db_dodelete: un_lock error");
}

static void _db_writedat(DB *db, const char *data, off_t offset, int whence)
{
  struct iovec   iov[2];
  static char    newline = NEWLINE;

  if (whence == SEEK_END)
    if (writew_lock(db->datfd, 0, SEEK_SET, 0) < 0)
      err_dump("_db_writedat: writew_lock error");
    
  if ((db->datoff = lseek(db->datfd, offset, whence)) == -1)
    err_dump("_db_writedat: lseek error");
  db->datlen = strlen(data) + 1;

  iov[0].iov_base = (char *) data;
  iov[0].iov_len = db->datlen - 1;
  iov[1].iov_base = &newline;
  iov[1].iov_len = 1;
  if (writev(db->datfd, &iov[0], 2) != db->datlen)
    err_dump("_db_writedat: writev error of data record");

  if (whence == SEEK_END)
    if (un_lock(db->datfd, 0, SEEK_SET, 0) < 0)
      err_dump("_db_writedat: un_lock error");
}


static void _db_writeidx(DB *db, const char *key, off_t offset, int whence, off_t ptrval)
{
  struct iovec  iov[2];
  char          asciiptrlen[PTR_SZ + IDXLEN_SZ + 1];
  int           len;

  if ((db-> ptrval = ptrval) < 0 || ptrval > PTR_MAX)
    err_quit("_db_writeidx: invalid ptr: %d", ptrval);
  sprintf(db->idxbuf, "%s%c%lld%c%ld\n", key, SEP, (long long)db->datoff, SEP, (long)db->datlen);
  len = strlen(db->idxbuf);
  if (len < IDXLEN_MIN || len > IDXLEN_MAX)
    err_dump("_db_writeidx: invalid length");
  sprintf(asciiptrlen, "%*lld%*d", PTR_SZ, (long long)ptrval, IDXLEN_SZ, len);

  if (whence == SEEK_END)
    if (writew_lock(db->idxfd, ((db->nhash + 1) * PTR_SZ) + 1, SEEK_SET, 0) < 0)
      err_dump("_db_writeidx: writew_lock error");

  if ((db->idxoff = lseek(db->idxfd, offset, whence)) == -1)
    err_dump("_db_writeidx: lseek error");

  iov[0].iov_base = asciiptrlen;
  iov[0].iov_len = PTR_SZ + IDXLEN_SZ;
  iov[1].iov_base = db->idxbuf;
  iov[1].iov_len = len;
  if (writev(db->idxfd, &iov[0], 2) != PTR_SZ + IDXLEN_SZ + len)
    err_dump("_db_writeidx: writev error of index record");

  if (whence == SEEK_SET)
    if (un_lock(db->idxfd, ((db->nhash + 1)*PTR_SZ)+1, SEEK_SET, 0) < 0)
      err_dump("_db_writeidx: un_lock error");
}

static void _db_writeptr(DB *db, off_t offset, off_t ptrval)
{
  char    asciiptr[PTR_SZ + 1];

  if (ptrval < 0 || ptrval > PTR_MAX)
    err_quit("_db_writeptr: invalid ptr %d", ptrval);
  sprintf(asciiptr, "%*lld", PTR_SZ, (long long)ptrval);

  if (lseek(db->idxfd, offset, SEEK_SET) == -1)
    err_dump("_db_writeptr: lseek error to ptr field");
  if (write(db->idxfd, asciiptr, PTR_SZ) != PTR_SZ)
    err_dump("_db_writeptr: write error to ptr field");
}

int db_store(DBHANDLE h, const char *key, const char *data, int flag)
{
  DB    *db = h;
  int   rc, keylen, datlen;
  off_t ptrval;

  if (flag != DB_INSERT && flag != DB_REPLACE && flag != DB_STORE) {
    errno = EINVAL;
    return (-1);
  }

  keylen = strlen(key);
  datlen = strlen(data) + 1;   /* +1 是为了末尾的 newline */

  if (datlen < DATLEN_MIN || datlen > DATLEN_MAX)
    err_dump("db_store: invalid data length");
  
  if (_db_find_and_lock(db, key, 1) < 0) {  /* record not found */
    if (flag == DB_REPLACE) {
      rc = -1;
      db->cnt_storerr++;
      errno = ENOENT;      /* error, record does not exist */
      goto doreturn;
    }

    /**
     * _db_find_and_lock locked the hash chain for us;
     * read the chain ptr to the first index record on hash table
     */
    ptrval = _db_readptr(db, db->chainoff);

    if (_db_findfree(db, keylen, datlen) < 0) {
      /**
       * Can't find an empty record big enough. Append the
       * new record to the ends of the index and data files.
       */
      _db_writedat(db, data, 0, SEEK_END);
      _db_writeidx(db, key, 0, SEEK_END, ptrval);

      /**
       * db->idxoff was set by _db_writeidx. The new
       * record goes to the front of the hash table.
       */
      _db_writeptr(db, db->chainoff, db->idxoff);
      db->cnt_stor1++;
    } else {
      /** 
       * Reuse an empty record. _db_findfree remove it from
       * the free list and set both db->datoff and db->idxoff.
       * Reused record goes to the front of the hash chain.
       */
      _db_writedat(db, data, db->datoff, SEEK_SET);
      _db_writeidx(db, key, db->idxoff, SEEK_SET, ptrval);
      _db_writeptr(db, db->chainoff, db->idxoff);
      db->cnt_stor2++;
    }
  } else {
    if (flag == DB_INSERT) {
      rc = 1;
      db->cnt_storerr++;
      goto doreturn;
    }

    if (datlen != db->datlen) {
      _db_dodelete(db);

      ptrval = _db_readptr(db, db->chainoff);

      _db_writedat(db, data, 0, SEEK_END);
      _db_writeidx(db, key, 0, SEEK_END, ptrval);

      _db_writeptr(db, db->chainoff, db->idxoff);
      db->cnt_stor3++;
    } else {
      _db_writedat(db, data, db->datoff, SEEK_SET);
      db->cnt_stor4++;
    }
  }
  rc = 0;

doreturn:
  if (un_lock(db->idxfd, db->chainoff, SEEK_SET, 1) < 0)
    err_dump("db_store: un_lock error");
  return (rc);
}

static int _db_findfree(DB *db, int keylen, int datlen)
{
  int        rc;
  off_t      offset, nextoffset, saveoffset;

  if (writew_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("_db_findfree: writew_lock error");

  saveoffset = FREE_OFF;
  offset = _db_readptr(db, saveoffset);
  while (offset != 0) {
    nextoffset = _db_readidx(db, offset);
    if (strlen(db->idxbuf) == keylen && db->datlen == datlen)
      break;
    saveoffset = offset;
    offset = nextoffset;
  }

  if (offset == 0) {
    rc = -1;
  } else {
    _db_writeptr(db, saveoffset, db->ptrval);
    rc = 0;
  }

  if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("_db_findfree: un_lock error");
  return (rc);
}

void db_rewind(DBHANDLE h)
{
  DB       *db = h;
  off_t    offset;

  offset = (db->nhash + 1) * PTR_SZ;

  /**
   * We're just setting the file offset for this process
   * to the start of the index records; no need to lock.
   * +1 below for newline at end of hash table.
   */
  if ((db->idxoff = lseek(db->idxfd, offset+1, SEEK_SET)) == -1)
    err_dump("db_rewind: lseek error");
}

char *db_nextrec(DBHANDLE h, char *key)
{
  DB     *db = h;
  char   c;
  char   *ptr;

  if (readw_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("db_nextrec: readw_lock error");

  do {
    if (_db_readidx(db, 0) < 0) {
      ptr = NULL;
      goto doreturn;
    }

    ptr = db->idxbuf;
    while ((c = *ptr++) != 0 && c == SPACE)
      ;
  } while (c == 0);

  if (key != NULL)
    strcpy(key, db->idxbuf);
  ptr = _db_readdat(db);
  db->cnt_nextrec++;

doreturn:
  if (un_lock(db->idxfd, FREE_OFF, SEEK_SET, 1) < 0)
    err_dump("db_nextrec: un_lock error");
  return (ptr);
}
