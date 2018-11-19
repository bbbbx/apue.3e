# Process Control

## 进程 ID

系统中有一些特殊的进程。PID 为 0 的通常是 scheduler process，也被称为 *swapper*，该进程是内核的一部分，是一个 system process。PID 为 1 的通常是 `init` 进程，在机器 bootstrap 结束的时候由内核调用。

之前该进程的 program file 是 `/etc/init`，现在在一些系统中是 `/sbin/init`。`init` 通常是一个带有 superuser privileges 的 user process，不是像 swapper 那样的 system process。

`init` 还是所有 orphaned child process 的父进程。

在 Mac OS X 10.4 中，`init` 进程被 `launchd` 进程代替。

每个 UNIX System implementation 都有它自己的 set of kernel processes，这些进程提供了 operating system services。例如，在一些 virtual memory implementations 中，PID 为 2 的是 *pagedaemon*，该进程负责 the virtual memory system 的 the paging。

```c
#include <unistd.h>

pid_t getpid(void);    /* 获取调用进程的 process ID */

pid_t getppid(void);   /* 获取调用进程的父进程的 process ID */

uid_t getuid(void);    /* 获取调用进程的 real user ID */

uid_t geteuid(void);   /* 获取调用进程的 effective user ID */

gid_t getgid(void);    /* 获取调用进程的 real group ID */

gid_t getegid(void);   /* 获取调用进程的 effective group ID */
```

注意，这些函数都没有错误返回值。

## `fork` Function

```c
#include <unistd.h>

pid_t fork(void);    /* 子进程返回 0，父进程返回子进程的 PID。 */
```

由于 `fork` 之后经常是 `exec`，所以现在的系统并不是完全地复制父进程的 data、stack 和 heap，而是使用了 *copy-on-write* 技术。

某些系统提供了 `fork` 的几种变体，本书讨论的 4 种系统都支持 `vfork`。

Linux 3.2.0 提供了 `clone` 函数。FreeBSD 8.0 提供了 `rfork` 系统调用。

一般来说，并不能确定在 `fork` 之后是父进程先执行还是子进程先执行，这取决于内核的 scheduling algorithm。

`strlen` 计算的字符串长度不包含终止 `null` 字节，而 `sizeof` 计算的缓冲区长度包含 `null` 字节。`sizeof` 是在编译的时候就计算缓冲区长度的。

### 文件共享

在重定向父进程的标准输出时，子进程的标准输出也被重定向了。

父进程和子进程共享一个文件的偏移量：如果父进程的标准输出已经重定向，那么子进程写到该标准输出时，它将会更新与父进程共享的该文件的偏移量。

除了 open file 之外，父进程的很多属性也会被子进程继承：

- Real user ID, real group ID, effective user ID, and effective group ID
- Supplementary group IDs
- Process group ID
- Session ID
- Controlling terminal
- The set-user-ID and set-group-ID flags
- 目前的 working directory
- Root directory
- File mode creation mask
- Signal mask and dispositions
- The close-on-exec flag for any open file descriptors
- Environment
- Attached shared memory segments
- Memory mappings
- Resource limits

父进程和子进程的不同点是：

- `fork` 的返回值不同
- PID 不同
- PPID 不同
- 子进程的 `tms_utime`、`tms_stime`、`tms_cutime` 和 `tms_cstime` 的值为 0（见 8.17）。
- 子进程不继承父进程的 file lock。
- 子进程的 pending alarms 会被清除。
- 子进程的 set of pending signal 会被设置为空集。

`fork` 失败的两个主要原因是：

1. 系统中已经有了太多的进程。
2. 该 real user ID 的进程总数超过了系统限制（图 2.11 中的`CHILD_MAX` 参数，我在 Mac OS X 10.14 中得到的值是 709）。

`fork` 有以下两种用法：

1. 当一个进程想要复制自己，使得父进程和子进程可以同时执行不同的 section of code 时。这种用法在 network servers 中很常见。当一个 request 到达的时候，父进程调用 `fork`，让子进程来处理该 request，而父进程则继续等待下一个 request 的到来。
2. 当一个进程想要执行另一个不同的 program 时。这在 shells 很常见。这时，子进程会做一个 `exec` 操作。

某些系统将一个 `fork` 和一个 `exec` 组合成一个单一的操作，称为一个 *spawn*。

## `vfork` Function

`vfork` 的原型和 `fork` 一样。

`vfork` 用于 创建一个新的进程，而该新的进程的目的就是为了执行一个新的 program，类似于 `fork` 的 shell 用法，但它并不会将父进程的 address space 复制给子进程，因为子进程只会简单地调用 `exec` 或 `exit`，所以子进程不会用到 address space。所以子进程是在父进程的 address space 中运行的。

`vfork` 还会保证子进程先运行，在它调用 `exec` 或 `exit` 之后，父进程才会继续运行（如果子进程依赖于父进程的 further action 的话，这会导致死锁的出现）。

## `exit` Function

一个进程可以有 5 种正常的 terminate 方式及 3 种异常的 terminate 方式。5 种正常的 terminate 方式：

1. `main` 函数中的 `return` 语句。
2. 调用 `exit` 函数。
3. 调用 `_exit` 或 `_Exit` 函数。
4. 在进程的 last thread 的 start routine 中执行 `return`。
5. 在进程的 last thread 中调用 `pthread_exit` 函数。

3 中异常的 terminate 方式：

1. 调用 `abort`。它会产生一个 `SIGABRT` signal。
2. 当进程收到某些 signals 时。
3. 进程的 last thread 对 cancellation request 作出了响应。

不管一个进程是如何 terminates 的，它都会执行内核中的同一段代码。该内核代码会关闭进程的所有 open descriptors、释放进程所使用的内存等。

如果一个父进程在子进程 terminate 前就 terminate 了，那会发生什么呢？答案是：对于父进程已经 terminate 的所有子进程，都将它们的父进程改为 `init` 进程，我们称这些进程是继承自 `init` 进程的。其操作过程大致是：在 terminate 一个进程时，内核会检查每个 active processes 看看是否是要 terminate 的进程的子进程，如果是，则将该进程的 PPID 该为 1。这样就保证了所有进程都有一个父进程。

在 UNIX 术语中，一个已经终止的、但其父进程尚未对其进行善后处理的（获取已经终止的子进程的有关信息、终止释放它占用的资源）进程被称称为 *zombie*。

## `wait` and `waitpid` Functions

当一个进程终止时，内核就会向其父进程发送 `SIGCHLD` 信号。

```c
#include <sys/wait.h>

pid_t wait(int *statloc);

pid_t waitpid(pid_t pid, int *statloc, int options);
```

这两个函数的表现如下：

- 如果调用该函数的进程还有子进程在运行，则阻塞调用该函数的进程。
- 如果有子进程终止，则调用该函数的进程带有其子进程的 termination status 立即返回。
- 如果调用该函数的进程没有子进程，则返回错误。

这两个函数的区别如下：

- 在一个子进程终止之前，调用 `wait` 的进程将会阻塞，而 `waitpid` 有一个 option 可以使其不阻塞。
- `waitpid` 并不等待第一个终止的子进程，它有很多个 options，因此可以选择它所想要等待的子进程。

参数 `statloc` 是一个指向 integer 的 pointer。如果该参数不是一个 null pointer，则终止的子进程的 termination status 就会存放在它所指向的地方。

有 4 个互斥的 macros 可以告诉我们进程是如何终止的，它们都以 `WIF` 开头。

`waitpid` 函数提供了 `wait` 函数没有提供的 3 个功能：

1. `waitpid` 可以等待一个指定的子进程，而 `wait` 等待第一个终止的子进程。
2. `waitpid` 可以使父进程非阻塞。
3. `waitpid` 可以用 `WUNTRACED` 和 `WCONTINUED` options 支持 job control。

## `waitid` Function

Single UNIX Specification 包括了一个额外用来获取进程的 exit status 的函数，`waitid` 类似于 `waitpid`，但更加地灵活。

## `wait3` 和 `wait4` 函数

这两个函数可以返回终止进程及其所有子进程使用的资源的概括。

## Race Conditions

当多个进程都想要对共享数据做些什么的时候，就会发生 race condition。

## exec Function

 当进程调用一种 exec 函数时，该进程执行的 program 完全替换为新的 program，而新的 program 则从其 `main` 函数开始执行。因为调用 exec 并不创建新的进程，所以前后的 PID 并未改变。exec 只是用 disk 上的一个 program 替换了当前进程的 text segment、data segment、heap segment 和 stack segment。

有 7 种不同的 exec 函数，它们统称为 exec  函数。

```c
#include <unistd.h>

int execl(const char *pathname, const char *arg0, ... /* (char *)0 */);

int execv(const char *pathname, char *const arg[]);

int execle(const char *pathname, const char *arg0, ... /* (char *)0, char *const envp[] */);

int execlp(const char *filename, const char *arg0, ... /* (char *)0 */);

int execvp(const char *filename, char *const argv[]);

int fexecve(int fd, char *const argv[], char *const envp[]);
```

当指定 `filename` 作为参数时：

- 如果 `filename` 中包含 `/`，则将其视为路径名。
- 否则就按 `PATH` 环境变量，在它所指定的各个目录中搜索可执行文件，目录之间用冒号隔开。

```bash
PATH=/bin:/usr/bin:/usr/local/bin:.
```

最后的路径前缀 `.` 表示当前目录。出于安全性的原因，*永远* 不要将当前目录加入到搜索路径中，see Garfinkel et al. [2003]。

## 更改用户 IDs 和组 IDs

```c
#include <unistd.h>

int setuid(uid_t uid);

int setgid(gid_t gid);

/* 成功则返回 0，出错返回 -1 */
```

FreeBSD 8.0 和 Linux 3.2.0 提供了 `setreuid` 和 `setregid`，它们分别可以用来获取保存的 set-user-ID 和保存的 set-group-ID。

```c
#include <unistd.h>

int setreuid(uid_t ruid, uid_t euid);

int setregid(gid_t rgid, gid_t egid);

/* 成功则返回 0，出错返回 -1 */
```

POSIX.1 包含了两个函数，可用来改变 effective user ID 和 effective group ID。

```c
#include <unistd.h>

int seteuid(uid_t uid);
int setegid(gid_t gid);
```

## Interpreter Files

所有的 UNIX 系统都支持 interpreter files。这些文件是以下面这行为开头的文本文件：

```txt
#! pathname [ optional-argument ]
```

其中，在感叹号和 *pathname* 之间的空格是可选的，常见的 interpreter file 这样开头：

```txt
#!/bin/bash
```

其中 *pathname* 是绝对路径名。实际上，被内核执行的不是该 interpreter file，而是 interpreter file 第一行的 pathname 所指定的文件。一定要将 interpreter file（以 `#!` 开头的文本文件）和 interpreter（由 *pathname* 指定的文件）区分开来。

## `system` Function

```c
#include <stdlib.h>

int system(char *cmdstring);
```

`system` 是通过调用 `fork`、`exec` 和 `waitpid` 来实现的，所以有 3 中返回值。

## Process Accounting

大多数 UNIX 系统都有一个选项来开启 process accounting。当开启这项功能的时候，每当有一个进程终止的时候，内核就会写入一个 accounting record。这些 accounting records 通常包括命令名、CPU 的使用时间、user ID 和 group ID、启动时间等等。

可以使用 `acct` 函数来开启 process accounting。`accton` 命令就使用到了该函数，superuser 带有一个路径名执行 `accton` 命令可以开启 accounting。Accounting records 被写入到指定的文件当中，在 FreeBSD 和 Mac OS X 中，该文件是 `/var/account/acct`。

## User Identification

有时候我们想要找出运行该 program 的用户的 login name。`getlogin` 函数可以取到该 login name。

```c
#include <unistd.h>

char *getlogin(void);  /* 出错则返回 NULL */
```

如果调用此函数的进程不是被 attached 在用户登录的 terminal 上时，则该函数可能会失败。我们通常称这些进程为 *daemons*。

## Process Scheduling

进程可以通过调整它的 “nice value”（通过调整它的 nice value 可以降低它对 CPU 的占用，所以该进程是 nice 的）来以一个较低的 priority 运行。

SUS 中 nice 值的范围在 0 ~ `(2*NZERO)-1` 之间。nice 值越大，优先度越低。也就是说你越 nice，你的调度优先级就越低。`NZERO`（在 Mac OS X 10.4 中为 20） 是系统默认的 nice 值。

进程可以通过 `nice` 函数来获取或更改它的 nice 值。

```c
#include <unistd.h>

int nice(int incr);   /* 成功返回新的 nice 值，失败返回 -1 */
```

`getpriority` 除了可以获取一个进程的 nice 值外，还可以获取一组相关进程的 nice 值。

```c
#include <sys/resource.h>

int getpriority(int which, id_t who);

int setpriority(int which, id_t who, int value);
```

`nice` 系统调用源于早期 Research UNIX 系统的 PDP-11 版本。`getpriority` 和 `setpriority` 函数源于 4.2BSD。

## Process Times

进程可以调用 `times` 函数来获取它自己的或者 any terminated children 的 wall clock time、user CPU time 和 system CPU time。

```c
#include <sys/times.h>

clock_t times(struct tms *buf);

struct tms {
  clock_t   tms_utime;    /* user CPU time */
  clock_t   tms_stime;    /* system CPU time */
  clock_t   tms_cutime;   /* user CPU time, terminated children */
  clock_t   tms_cstime;   /* system CPU time, terminated children */
};
```
