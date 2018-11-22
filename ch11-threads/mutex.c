/**
 * 在一个 data structure 内
 * 使用一个 mutex 来保护该 data structure。
 */

#include <stdlib.h>
#include <pthread.h>

struct foo {
  int             f_count;
  pthread_mutex_t f_lock;
  int             f_id;
  /* 下面可以放更多的 stuff */ 
};

struct foo *foo_alloc(int id)   /* allocate 该 object */
{
  struct foo *fp;

  if ((fp = malloc(sizeof(struct foo))) != NULL) {
    fp->f_count = 1;
    fp->f_id = id;
    if (pthread_mutex_init(&fp->f_lock, NULL) != 0) {
      free(fp);
      return(NULL);
    }
    /* ... 其他的初始化操作 ... */ 
  }
  return(fp);
}

void foo_hold(struct foo *fp)   /* add 一个 reference 到该 object 上  */
{
  pthread_mutex_lock(&fp->f_lock);
  fp->f_count++;
  pthread_mutex_unlock(&fp->f_lock);
}

void foo_rele(struct foo *fp)   /* release 该 object 的一个 reference  */
{
  pthread_mutex_lock(&fp->f_lock);
  if (--fp->f_count == 0) {     /* 最后一个 reference，没有人 refer 了，destroy 该 mutex */
    pthread_mutex_unlock(&fp->f_lock);
    pthread_mutex_destroy(&fp->f_lock);
    free(fp);
  } else {
    pthread_mutex_unlock(&fp->f_lock);
  }
}
