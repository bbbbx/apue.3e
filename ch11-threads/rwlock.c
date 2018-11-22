/**
 * 一个 queue of job 的 requests
 * 由一个 reader–writer lock 来保护。
 * 多个 worker threads 从
 * 一个 master thread 中
 * 获取分配给它们的 job。
 */

#include "apue.h"
#include <pthread.h>

struct job {
  struct job  *j_next;
  struct job  *j_prev;
  pthread_t   j_id;    /* 用来指定哪个线程处理这个 job */
  /* ... 下面可以添加更多的信息 ... */
};

struct queue {
  struct job          *q_head;
  struct job          *q_tail;
  pthread_rwlock_t    q_lock;
};

/**
 * 初始化一个 queue
 */
int queue_init(struct queue *qp)
{
  int err;

  qp->q_head = NULL;
  qp->q_tail= NULL;
  err = pthread_rwlock_init(&qp->q_lock, NULL);
  if (err != 0)
    return (err);
  
  return (0);
}

/**
 * 插入一个 job 到 queue 的 head 中
 */
void job_insert(struct queue *qp, struct job *jp)
{
  pthread_rwlock_wrlock(&qp->q_lock);
  jp->j_next = qp->q_head;
  jp->j_prev = NULL;
  if (qp->q_head != NULL)
    qp->q_head->j_prev = jp;
  else
    qp->q_head->j_prev = jp;   /* list was empty */
  qp->q_head = jp;
  pthread_rwlock_unlock(&qp->q_lock);
}

/**
 * 在 queue 的 tail 上 append 一个 job
 */
void job_append(struct queue *qp, struct job *jp)
{
  pthread_rwlock_wrlock(&qp->q_lock);
  jp->j_next = NULL;
  jp->j_prev = qp->q_tail;
  if (qp->q_tail != NULL)
    qp->q_tail->j_next = jp;
  else
    qp->q_head = jp;    /* list was empty */
  qp->q_tail = jp;
  pthread_rwlock_unlock(&qp->q_lock);
}

/**
 * 从一个 queue 中移除指定的 job
 */
void job_remove(struct queue *qp, struct job *jp)
{
  pthread_rwlock_wrlock(&qp->q_lock);
  if (jp == qp->q_head) {
    qp->q_head = jp->j_next;
    if (qp->q_tail == jp)
      qp->q_tail = NULL;
    else
      jp->j_next = NULL;
  } else if (jp == qp->q_tail) {
    qp->q_tail = jp->j_prev;
    jp->j_prev->j_next = jp->j_next;
  } else {
    jp->j_prev->j_next = jp->j_next;
    jp->j_next->j_prev = jp->j_prev;
  }
  pthread_rwlock_unlock(&qp->q_lock);
}

/**
 * 对指定的 thread ID 找到一个 job
 */
struct job *job_find(struct queue *qp, pthread_t id)
{
  struct job *jp;

  if (pthread_rwlock_rdlock(&qp->q_lock) != 0)
    return (NULL);

  for (jp = qp->q_head; jp != NULL; jp = jp->j_next)
    if (pthread_equal(jp->j_id, id))
      break;

  pthread_rwlock_unlock(&qp->q_lock);
  return (jp);
}
