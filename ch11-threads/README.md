# Threads

可以在一个进程内使用多个 *threads of control*（或简称为 *threads*） 来完成多项 tasks。一个进程内的所有线程都可以访问该进程的 components， 例如 file descriptors 和 memory。

## Thread Concepts

进程内的每个线程都包含一些表示该线程的一些信息，包括一个 *thread ID*、一组 register value、一个 stack、一个 scheduling priority and policy、一个 signal mask、一个 `errno` 变量和一些线程特有的数据。

一个进程的所有信息对于该进程内的所有线程来说都是 sharable 的，包括 text of the executable program、进程的 global and heap memory、the stack 和 file descriptors。

进程的接口是从 POSIX.1-2001 中来的，称为 "pthreads"（即 POSIX threads）。POSIX threads 的 feature test macro 是 `_POSIX_THREADS`。Application 可以用一个 `#ifdef` 来在 compile time 测试是否支持线程，或者用 `_SC_THREADS` 常量来调用 `sysconf` 函数在 runtime 测试是否支持线程。

## Thread Identification

线程可以通过调用 `pthread_self` 函数获得自己的 thread ID。

```c
#include <pthread.h>

pthread_t pthread_self(void);
```

其中的 `pthread_t` 是一个 structure，所以不能简单地像 `pid_t` 那样进行比较，因此需要一个函数来对两个 thread ID 进行比较。

```c
#include <pthread.h>

int pthread_equal(pthread_t tid1, pthread_t tid2);
```

## Thread Creation

在传统的 UNIX process model 中，每个进程只支持一个 thread of control。在 POSIX threads 中，当一个 program 运行的时候，它也是只有一个 thread of control，然后它可以创建多个 threads of control。

```c
#include <pthread.h>

int pthread_create(pthread_t *restrict tidp, const pthread_attr_t *restrict attr, void *(*start_rtn)(void *), void *restrict arg);
```

`attr` 参数用来指定各种 thread attributes。新建的进程从 `start_rtn` 函数的地址开始运行，而 `arg` 是该函数的 argument。

当创建了一个新的进程后，并不保证哪个线程会被先执行：是新建的线程会被先执行还是 the calling thread 会被先执行。新建的线程可以访问进程的 address space，并且继承了 the calling thread 的 floating-point environment 和 signal mask，而该线程的 pending signals 会被清空。

## Thread Termination

一个进程内的任何一个线程如果调用了 `exit`、`_Exit` 或 `_exit`，那么整个进程都会 terminates 掉。

单个线程可以通过 3 种方式退出，而不是终止掉整个进程。

1. 从线程的 the start routine 中 return。返回值是该线程 exit code。
2. 一个线程可以被同个进程内的另一个线程取消。
3. 该进程调用 `pthread_exit` 函数。

```c
#include <pthread.h>

void pthread_exit(void *rval_ptr);
```

其中的 `rval_ptr` 参数与传给 the start routine 的那个参数类似。该 pointer 可以给进程内的其他线程通过 `pthread_join` 函数访问到。

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **rval_ptr);
```

线程可以通过调用 `pthread_cancel` 函数来请求另一个线程被 canceled。

```c
#include <pthread.h>

int pthread_cancel(pthread_t tid);
```

默认情况下，`pthread_cancel` 函数等价于 `tid` 标识的线程调用了 `pthread_exit(PTHREAD_ANCELED)` 函数，但是，线程可以选择忽略该 cancellation 或控制如何被 canceled。

线程也可以有它自己的 *thread cleanup handler*，这和进程可以用 `atexit`  函数（Section 7.3）来决定在进程 exit 时可以调用哪些函数是类似的。

```c
#include <pthread.h>

void pthread_cleanup_push(void (*rtn)(void *), void *arg);

void pthread_cleanup_pop(int execute);
```

thread functions 和 process functions 的相识之处：

|Process primitive|Thread primitive|Description                  |
|:----------------|:---------------|:----------------------------|
|`fork`           |`pthread_create`|创建一个新的 flow of control   |
|`exit`           |`pthread_exit`  |从一个已存在的 flow of control 中 exit |
|`waitpid`        |`pthread_join`  |获取一个 flow of control 的 exit status |
|`atexit`         |`pthread_cleanup_push` |注册一个在 flow of control exit 的时候可以调用的函数|
|`getpid`         |`pthread_self`         |获取 flow of control 的 ID |
|`abort`          |`pthread_cancel`       |请求对一个 flow of control 进行 abnormal termination |

可以调用 `pthread_detach` 函数来 detach 一个线程。

```c
#include <pthread.h>

int pthread_detach(pthread_t tid);
```

## Thread Synchronization

当有多个 threads of control 共享了相同的 memory 时，我们就需要确保每个进程看到的数据都是 consistent 的。

当一个进程修改了某个变量，这样其他的进程在读取这个变量的时候看到的数据就可能是 inconsistent 的。在 processor architectures 内，如果修改一个变量的值花费的时间比一个 memory cycle 的时间还要长的话，那么进程之间看到的数据就是 inconsistent 的，例如在两个 memory write cycles 的中间插入了一个 memory read（假设 write 花费的时间需要两个 CPU 周期）。

### Mutexes（Mutual exclusion，互斥锁）

我们可以用 pthreads mutual-exclusion 接口确保我们的数据在某个时刻只有一个线程在访问。一个 *mutex* 类似于一把 lock，在访问一个 shared resource 之前，我们先 set（lock），当我们 done 的时候，再 release (unlock) 。当一个 shared resource 被 set 了之后，任何其他想要 set 该 shared resource 的线程都会被 blocked，直到我们 release 该 shared resource 为止。

一个 mutex variable 使用 `pthread_mutex_t` 数据类型来表示。在使用一个 mutex variable 之前，我们必须要先对它进行初始化。如果是动态分配 mutex 的（例如通过 `malloc` 来分配），则我们需要在 freeing 该 memory 之前调用 `pthread_mutex_destroy` 函数。

```c
#include <pthread.h>

int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);

int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

我们可以调用 `pthread_mutex_lock` 函数来 lock 一个 mutex。如果该 mutex 已经被 locked，则调用该函数的线程会被 blocked，直到该 mutex 被 unlocked。调用 `pthread_mutex_unlock` 函数可以 unlock 一个 mutex。

```c
#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

如果该线程不想被 blocked，则可以调用 `pthread_mutex_trylock`，如果该 mutex 已经被 locked，则直接返回 `EBUSY`。

### Deadlock Avoidance

如果一个线程想要 lock 两次相同的 mutex 时，那么该线程就会陷入死锁。如果两个线程都想要另一方已经 lock 住的 mutex，则这两个线程谁都无法继续执行，因此也会出现死锁。

可以通过控制 mutexes 的 lock order(加锁的顺序)来避免死锁。

但是，如果你加锁的粒度太粗，就会出现很多线程为了同一个 metux 而被 blocked 住，影响了并发性；如果你加锁的粒度太细，那过多的加锁操作就会影响性能，且使你的代码变得复杂。在 code complexity 和 performance 之间找到一个 correct balance 是 programmer 需要做的。

### `pthread_mutex_timedlock` Function

可以使用 `pthread_mutex_timedlock` 来避免线程被永久 blocking，如果超时了还没有 lock 住 mutex，则会返回一个 error code `ETIMEOUT`。

```c
#include <pthread.h>
#include <time.h>

int pthread_mutex_timedlock(pthread_mutex_t *restrict mutex, const struct timespec *restrict tsptr);
```

超时时间是以绝对时间来指定的。

<small>Mac OS X 10.14 还不支持该函数，但 FreeBSD 8.0、Linux 3.2.0 及 Solaris 10 都支持该函数</small>

### Reader–Writer Locks（读写锁）

Reader–writer locks 类似于 mutexes，不过读写锁可以让进程具有高度的并行性。

在一个 mutex 中，它的 state 不是 locked 就是 unlocked，且某一时刻只能让一个线程 lock 住它。而一个 reader–writer lock 可以有 3 种 states：locked in read mode、locked in write mode 和 unlocked。某一时刻只能让一个线程持有一个 write mode 的 reader–writer lock，不过可以让多个线程持有一个 read mode 的 reader–writer lock。

Reader–writer locks 非常适合那些读的次数多于修改的次数的数据结构。

读写锁有时也称为 shared–exclusive locks（互斥共享锁）。当读写锁处于 read locked 时，可以看做是 shared mode，当读写锁处于 write locked 时，可以看做是 exclusive mode。

```c
#include <pthread.h>

int pthread_rwlock_int(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attr);

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

SUS 还定义了 reader–writer locking 的 conditional versions。

```c
#include <pthread.h>

int pthread_rwlock_tryrdlock(pthread_rwlock *rwlock);

int pthread_rwlock_trywrlock(pthread_rwlock *rwlock);
```

### Reader–Writer Locking with Timeouts

`pthread_rwlock_timedrdlock` 函数和 `pthread_rwlock_timedwrlock` 函数。

### Condition Variables（条件变量）

Condition variables 是线程的另一种 synchronization mechanism。

The condition 本身是由一个 mutex 保护的。一个线程必须先 lock 住一个 mutex 才能改变 the condition 的 state。

### Spin Locks（自旋锁）

一个 spin lock 类似于一个 mutex，不过它不是通过 sleeping 来 block 一个进程的，而是通过 busy-waiting（spinning）。 当 lock 被持有的时间很短，且不想要花费太多的时间在 threads 的被 descheduled 上时，可以使用自旋锁。即lock 的持有时间比调度的时间短的时候，我们可以不让线程进行 sleeping，而是让 CPU 对它进行空转，这样就可以避免在 wake up 线程上花费的时间。

Spin locks 通常用来实现其他类型的 lock。在 user level 中，spin locks 不是那么有用。

### Barriers（屏障）

Barriers 是用来协调多个并行的线程的一种 synchronization mechanism。它可以让每个线程都处于 wait 的状态，直到所有的 cooperating threads 都到达了某种状态。`pthread_join` 函数就是一种狭义的 barrier，它可以让一个线程处于 wait 状态，直到另一个线程 exit。

```c
#include <pthread.h>

int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned int count);

int pthread_barrier_destroy(pthread_barrier_t *barrier);

int pthread_barrier_wait(pthread_barrier_t *barrier);
```

调用 `pthread_barrier_wait` 函数的线程会在 the barrier count（调用 `pthread_barrier_init` 函数时设置的） 没有被满足之前进行 sleeping。
