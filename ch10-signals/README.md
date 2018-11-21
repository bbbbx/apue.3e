# Signals

Signals 是 software interrupts。大多数复杂的 application programs 都需要处理 signals。
Signals 提供了一种处理 asynchronous events 的方法。

早期的 UNIX 系统就提供了 signals 功能，例如 UNIX Version 7，不过该 signal model 不是 reliable 的。4.3BSD 和 SVR3 改变了 signal model，添加了我们称为 *reliable signals* 的东西。POSIX.1 标准化了 reliable signals。

## 信号的一些概念

每个信号有一个 name，以字符 `SIG` 开头命名。UNIX Version 7 有 15 种信号，SVR4 和 4.4BSD 有 31 种。FreeBSD 8.0 有 32 种，Mac OS X 10.6.8 和 Linux 3.2.0 都有 31 种。Solaris 10 有 40 种。

信号的 name 都在头文件 `<signal.h>` 中被定义为 positive integer constants（signal number）。

<small>实际上，implementation 真正定义 signals 的头文件是另一个头文件，不过该头文件被 included 在 `<signal.h>` 中。在 FreeBSD 8.0 和 Mac OS X 10.6.8 中，该头文件是 `<sys.signal.h>`；在 Linux 3.2.0 中，该头文件是 `<bits/signum.h>`；在 Solaris 10 中，该头文件是 `<sys/ios/signal_ios.h>`</small>

没有信号的 signal number 是 0，0 signal number 对 `kill` 函数有特殊的用途（Section 10.9），POSOX.1 称这个值为 *null signal*。

|Name       |Description      |Default actioin    |
|:----------|:----------------|:------------------|
|SIGABRT    |异常的 termination（`abort`）|terminate+core|
|SIGALRM    |timer 的时间到了（`alarm`)   |terminate     |
|SIGBUS     |hardware fault             |terminate+core|
|SIGHUP     |hangup                     |terminate     |
|SIGINFO    |从 keyboard 来的 status request|ignore     |
|SIGINT     |terminal interrupt character  |terminate  |
|SIGQUIT    |terminal quit character       |terminate+core|
|SIGKILL    |termination                   |terminate  |
|SIGSTOP    |stop                          |stop process|
|SIGTERM    |termination                   |terminate   |
|...        |...                           |...         |

其中 Default actioin 中的 core 表示是否会产生 core 文件。

<small>core 文件是进程的 current working directory 下一个名为 `core` 的文件，该文件的内容是进程的 memory image（从该文件的文件名中可以看出这个 feature 成为 UNIX System 的 one part 已经好久了）。这个文件可以给大多数的 UNIX System debuggers 用来查看进程被 terminated 时的 state。不同 implementations 中的 core 文件名也不一样，FreeBSD 8.0 是 `cmdname.core`，其中 cmdname 是进程所执行的命令名，Mac OS X 10.6.8 是 `core.pid`，其中 pid 是进程的 PID，而 Linux 3.2.0 的 core 文件名是通过 `/proc/sys/kernel/core_pattern` 文件进行配置的。大多数的 implementations 的 core 文件都是在 current working directory 中，而 Mac OS X 将所有的 core 文件都放在 `/cores` 目录下。</small>

## `signal` Function

```c
#include <signal.h>

void (*signal(int signo, void (*func)(int)))(int);
```

其中 `signo` 参数是 signal name，`func` 的值可以是：

1. 常量 `SIG_IGN`：告诉系统忽略此信号（不能忽略 `SIGKILL` 和 `SIGSTOP`）。
2. 常量 `SIG_DFL`：告诉系统采取出现该信号时的 default action。
3. 一个函数的地址：我们称这种处理方法为 "catch" the signal，而该函数称为 *signal handler* 或 *signal-catching function。

`signal` 函数的返回值是一个函数的地址，该函数有一个 integer 参数（即最后的 `(int)`）。

`signal` 函数的原型太复杂了，可以使用下面的 `typedef`[Plauger, P.J. 1992.*The Standard C Library.*] 来简化一下：

```c
typedef void Sigfunc(int);
```

然后可以将 `signal` 函数的原型写成：

```c
Sigfunc *signal(int, Sigfunc *);
```

## Reentrant Functions

SUS 声明的某些函数可以保证在 signal handler 内被调用是 safe 的。这些函数是 reentrant 的，被 SUS 称为 *async-signal safe*。

下面是这些 async-signal safe 函数，还有很多没有列出。

|           |           |           |          |           |
|:----------|:----------|:----------|:---------|:-----------|
|`abort`    |`faccessat`|`linkat`   |`select`  |`socketpair`|
|`accept`   |`fchmod`   |`listen`   |`sem_post`|`stat`      |
|`access`   |`fchmodat` |`lseek`    |`send`    |`symlink`   |
|`alarm`    |`mkdir`    |`time`     |`open`    |...         |

## SIGCLD Semantics

`SIGCLD` 信号和 `SIGCHLD` 信号的区别。`SIGCLD`（没有 H)是从 UNIX System V 来的，而 `SIGCHLD` 是从 BSD 来的。POSIX.1 signal 也命名为 `SIGCHLD`。

当 一个进程 terminates 或 stops 的时候，`SIGCHLD` signal 就会发送给父进程。默认情况下，会忽略该信号。

## Reliable-Signal 的术语和语法

我们先定义一些在讨论 signals 时会用到的术语。首先，当有 event 造成一个 signal 出现时，一个 signal 是为了一个进程 *generated* 的（或是是 send 给父进程）。该 event 可以是一个 hardware exception（例如某数除以 0）、一个 software condition（例如一个 `alarm` 函数到时间了）、一个 terminal-generated signal 或者是对 `kill` 函数的调用。当一个 signal 被 generated 后，kernel 就会在 process table 中以某种 form 来 set 上一个 flag。

当一个 signal 的 action 被 taken 后，我们就说有某个进程的该 signal 是 *delivered* 的。在一个 signal 的 generation 和它的 delivery 这段时间内，我们就说该 signal 是被 *pending* 的。

进程有个 option 可以 *blocking* 一个 signal 的 delivery。进程可以调用 `sigpending` 函数（Section 10.13）来查看哪些 signals 是 blocked 的，哪些 signals 是 pending 的。

每个进程都有一个 *signal mask*，它定义了该进程目前 block 掉了哪些 signal 的 delivery，也就是一个 signals 的集合，该集合内的 signals 都不可以被 delivered。进程可以调用 `sigprocmask` 函数（Section 10.12）来查看或修改它当前的 signal mask。

因为 signal number 可能会超出一个 integer 的位数，所以 POSIX.1 定义了一个称为 `sigset_t` 的数据类型，它可以放进一个 *signal set*（Section 10.11）。

## `kill` 和 `raise` Function

```c
#include <signal.h>

/* 成功返回 0，出错返回 -1 */
int kill(pid_t pid, int signo);

int raise(int signo);
```

调用 `raise(signo);` 等价于调用 `kill(getpid(), signo);`。

## `alarm` 和 `pause` Function

`alarm` 函数可以让我们设置一个 timer，它会在一个指定的时间点 expire。如果一个 timer 已经 expires 的话，则就会 generate 一个 `SIGALRM` 信号。

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
```

`pause` 函数可以 suspends 调用它的进程，直到该进程 caught 到一个信号。

```c
#include <unistd.h>

int pause(void);
```

只有执行一个 signal handler 并返回，`pause` 才会返回。

## Signal Sets

POSIX.1 定义了数据类型 `sigset_t` 可以表示多个 signals（即一个 *signal set*），并定义了下列 5 个处理 signal set 的函数。

```c
#include <signal.h>

int sigemptyset(sigset_t *set);

int sigfillset(sigset_t *set);

int sigaddset(sigset_t *set, int signo);

int sigdelset(sigset_t *set, int signo)
/* 上面 4 个函数成功返回 0，失败返回 -1 */

int sigismember(const sigset_t *set, int signo);
```

`sigemptyset` 函数初始化由 `set` 指向的 signal set，清除其中所有的 signals，`sigfillset` 函数则相反。所有的 applications 在使用每个 signal set 之前都需要对 signal set 调用一次 `sigemptyset` 或 `sigfillset` 来初始化 signal set。

## `sigprocmask` Function

进程的 signal mask 是一个 signal set，该 signal set 里的 signals 会被 blocked 掉。进程可以通过调用 `sigprocmask` 函数来查看它的 signal mask、改变它的 signal mask。

```c
#include <signal.h>

int sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oset);
```

## `sigpending` Function

该函数返回一个 signal set，该 set 包含进程目前 pending 的 signals。

```c
#include <signal.h>

int sigpending(sigset_t *set);
```

## `sigaction` Function

`sigaction` 函数可以让我们查看或修改与指定的 signal 相关的 action。该函数取代了早期 UNIX 系统的 `signal` 函数。

```c
#include <signal.h>

int sigaction(int signo, const struct sigaction *restrict act, struct sigaction *restrict oact);
```

如果 `act` 参数不是 null，则就是修改该 signal 的 action。如果 `oact` 不是 null，则系统就会将之前的 action 返回给 `oact` pointer 指向的地方。其中的 `sigaction` structure 如下。

```c
struct sigaction {
  void         (*sa_handler)(int);   /* 其值为 signal handler 的地址或 SIG_IGN 或 SIG_DFL */

  sigset_t     sa_mask;     /* 用来 block 的额外 signals */
  int          sa_flags;    /* signal option，见图 10.16 */
  /* alternate handler */
  void         (*sa_sigaction)(int, siginfo_t *, void *);
};
```

## `sigsetjmp` and `siglongjmp` Function

在 Section 7.10 中，我们讨论了 `setjmp` 和 `longjmp` 函数，它们可以用在 nonlocal branching 上。我们经常会在一个 signal handler 里调用 `longjmp` 函数来返回一个 program 的 `main` loop 中，而不是从 signal handler 中返回。

但是，在 signal handler 中直接调用 `longjmp` 有一个问题。实际上，当 caught 到一个 signal 的时候，不但会调用 signal-catching 函数，而该 signal 也会自动地被添加到进程的 signal mask 中，这可以防止之后产生同样的 signal 时会中断该 signal handler。所以，如果我们只是简单在 signal handler 里调用 `longjmp`  函数，但不对进程的 signal mask 进行处理，这样就不对了。

POSIX.1 并没有说明 `setjmp` 和 `longjmp` 会对 signalmasks 进行处理，而是定义了两个新的函数：`sigsetjmp` 和 `siglongjmp`。当在一个 signal handler 中进行 nonlocal branching 的时候，就应该使用这两个函数。

```c
#include <setjmp.h>

int sigsetjmp(sigjmp_buf env, int savemask);  /* 若直接调用则返回零，若在 siglongjmp 里调用则返回非零 */

void siglongjmp(sigjmp_buf env, int val);
```

## `sigsuspend` Function

我们可以改变进程的 signal mask 来保护哪些我们不想被 signal 中断的 critical regions of code。

## `abort` Function

该函数可以造成 abnormal program termination。

```c
#include <stdlib.h>

void abort(void);
```

该函数将 `SIGABRT` signal 发送给调用该函数的进程（进程不应该 ignore 该 signal）。

## `system` Function

POSIX.1 要求 `system` 函数 ignore `SIGINT` 和 `SIGQUIT` 并 block 掉 `SIGCHLD`。

## `sleep`, `nanosleep` and `clock_nanosleep` Function

```c
#include <unistd.h>

unsigned int sleep(unsigned int seconds);
```

`nanosleep` 函数和 `sleep` 函数类似，但它的精度是纳秒级的（nanosecond-level）。

```c
#include <time.h>

int nanosleep(const struct timespec *reqtp, struct timespec *remtp);
```

而 `clock_nanosleep` 函数是相对于 system clocks（Section 6.10）的。


## `sigqueue` Function

有些系统支持 queueing signals。即除了 signal 本身携带的 one bit of information 外，在 deliver signal 的时候还可以带上一些额外的信息。

## Job-Control Signals

POSIX.1 将下列六个 signals 当作是 job-control signals：

`SIGCHLD`

`SIGCONT`

`SIGSTOP`

`SIGTSTP`

`SIGTTIN`

`SIGTTOU`

## Signal Names and Numbers

介绍如何在 signal number 和 signal name 之间自定义映射规则。

