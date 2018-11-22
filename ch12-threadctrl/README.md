# Thread Control

## Thread Limits

SUS 定义了几个和操作线程有关的 system limits，这些限制没有在 Figure 2.11 中列出来。就像其他的 system limits 一样，这些限制可以使用 `sysconf` 函数来查询。

|Name of limit|Description|`name` argument|
|:------------|:----------|:--------------|
|PTHREAD_DESTRUCTOR_ITERATIONS|一个线程 exits 时，系统尝试销毁线程数据的最大次数|_SC_THREAD_DESTRUCTOR_ITERATIONS|
|PTHREAD_KEYS_MAX   |进程可以创建的 keys 的最大数目|_SC_THREAD_KEYS_MAX|
|PTHREAD_STACK_MIN  |一个线程的 stack 的最小可用 bytes 数|_SC_THREAD_STACK_MIN|
|PTHREAD_THREADS_MAX|进程可以创建的最大线程数|_SC_THREAD_THREADS_MAX|

## Thread Attributes

线程的属性通过 `pthread_attr_t` structure 来表示，可以通过 `pthread_attr_init` 函数来初始化线程的属性。

```c
#include <pthread.h>

int pthread_attr_init(pthread_attr_t *attr);

int pthread_attr_destroy(pthread_attr_t *attr);
```

下面是 POSIX.1 定义的线程属性。

|Name      |Description|
|:---------|:----------|
|*detachstate* |detached thread attribute|
|*guardsize*   |guard buffer size in bytes at end of thread stack|
|*stackaddr*   |lowest address of thread stack|
|*stackaddr*   |minimum size in bytes of thread stack|

在 Section 11.5 中，我们介绍了 detached threads 的概念。如果我们对一个正在 exiting 线程的 termination status 不感兴趣的话，那我们可以使用 `pthread_detach` 函数来让系统在线程退出时回收它所占用的资源。

我们可以修改线程的属性，让线程一开始就处于 detached state。

```c
#include <pthread.h>

int pthread_attr_getdetachstate(const pthread_attr_t *restrict attr, int *detachstate);

int pthread_attr_setdetachstate(pthread_attr_t *attr, int *detachstate);
```

## Synchronization Attributes

线程的 synchronization objects 也有属性。包括 mutexes 的属性、reader–writer locks 的属性、condition variables 的属性和 barriers 的属性。

## Reentrancy

在 Section 10.6 中，我们讨论了 reentrant functions 和 signal handlers。线程遇到 reentrancy 问题的时候和 signal handlers 是类似的。因为可能会有多个线程同时调用同一个函数。

如果一个函数可以同时被多个线程进行 safely called，那么这个函数就是 *thread-safe* 的。

## Thread-Specific Data

Thread-specific data 也称为 thread-private data，只能在特定的线程内存储和查询线程特有的数据，这种数据不用担心和其他线程的同步问题。

其实，一个进程内所有的线程都可以访问该进程的 address space，所以，阻止一个线程去访问另一个线程的数据是不可能的，thread-specific data 也不例外。管理 thread-specific data 的函数只是让另一个线程访问一个线程的 thread-specific data 变得困难些。

在 allocating thread-specific data 之前，我们需要创建一个和 data 关联和 *key*。我们可以使用该 key 来访问 thread-specific data。

```c
#include <pthread.h>

int pthread_key_create(pthread_key_t *keyp, void (*destructor)(void *));
```

还可以对所有的线程 break 掉一个 key 和 the thread-specific data 的关联。

```c
#include <pthread.h>

int pthread_key_delete(pthread_key_t key);
```

## Cancel Options

有两个 thread attributes 并不包括在 `pthread_attr_t` structure 中，它们是 *cancelability* 和 *cancelability type*。这两个属性会影响到线程调用 `pthread_cancel` 函数时，线程对这个函数的响应行为（Section 11.5）。

## Threads and Signals

每个线程都有自己的 signal mask，但一个线程如何处理 signal 在进程内是被 shared 的。

Signals 是被 delivered 到进程内一个指定的线程中的。如果该 signal 和一个 hardware fault 有关，则该 signal 通常通常会被发送到引起该 event 的线程中。而其他的 signals，则有可能会发送到任意一个线程中。

线程可以使用 `pthread_sigmask` 函数来 block 掉 signals。

```c
#include <signal.h>

int pthread_sigmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
```

线程可以调用 `sigwait` 来等待一个或多个 signals 的出现。

```c
#include <signal.h>

int sigwait(const sigset_t *restrict set, int *restrict signop);
```

要把一个 signal 发送给一个进程，可以调用 `kill` 函数（Section 10.9）。而发送一个 signal 给线程，可以调用 `pthread_kill` 函数。

```c
#include <signal.h>

int pthread_kill(pthread_t thread, int signo);
```

## Threads and `fork`

在父进程的线程调用 `fork` 函数之后，在子进程内部，只有一个线程，它是父进程中调用 `fork` 函数的线程的 copy。

子进程的 address space 是父进程的 address space 的 copy，所以，子进程也继承了父进程的 mutex、reader–writer lock 和 condition variable 它们各自的 state。因此，在子进程内需要清除这些 lock 的 state。

要清除 lock 的 state，可以通过调用 `pthread_atfork` 函数来建立一个 *fork handlers*。

```c
#include <pthread.h>

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
```



## Thread and I/O

看 Section 3.11 中的 `pwrite` 和 `pread`。进程内的所有线程共享所有的 file descriptors。