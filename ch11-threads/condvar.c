/**
 * Condition 是 work queue 的 state，
 * 我们用一个 mutex 来保护该 condition，
 * 并在一个 while loop 中判断该 condition。
 */

#include <pthread.h>

struct msg {
  struct msg *m_next;
  /* ... more stuff here ... */
};

struct msg *workq;

pthread_cond_t qready = PTHREAD_COND_INITIALIZER;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

void process_msg(void)
{
  struct msg *mp;

  for (;;) {
    pthread_mutex_lock(&qlock);
    while (workq == NULL)
      pthread_cond_wait(&qready, &qlock);
    mp = workq;
    pthread_mutex_unlock(&qlock);
    /* 现在可以 process 该 message mp 了 */
  }
}

void enqueue_msg(struct msg *mp)
{
  pthread_mutex_lock(&qlock);
  mp->m_next = workq;
  workq = mp;
  pthread_mutex_unlock(&qlock);
  pthread_cond_signal(&qready);
}
