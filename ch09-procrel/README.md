# Process Relationships

## Terminal Logins

使用 terminal 登录到 host 的过程？

随着 bitmapped graphical terminals 的发展，现在很少有 terminal 这种物理设备了，windowing systems 给 users 提供了一种与 host computers 交互的新方法。现在也可以开发出 “terminal windows” 这样的 application（即 Mac 上的 “terminal” 应用？）来模拟 character-based terminals，这样就可以让 users 回到和熟悉的交互方式（即通过 shell 命令行）（然而对于一开始接触电脑就是 GUI 界面的我们来说并不熟悉）。

## Process Groups

每个进程除了有一个 PID 之外，还属于一个 process group。一个 process group 是一个或多个进程的集合，这些进程通常会合作完成同一个 job（job control 在 Section 9.8）。

函数 `getpgrp` 返回调用该函数的进程的 process group ID。

```c
#include <unistd.h>

pid_t getpgrp(void);

pid_t	 getpgid(pid_t pid);  /* 成功返回进程组 ID，失败返回 -1 */
```

SUS 定义了 `getpgid` 函数，`getpgid(0)` 等价于 `getpgrp()`。

每个 process group 都有一个 process group leader。该 leader 的 process group ID  等于它的 PID。

一个进程可以通过调用 `setpgid` 函数加入或离开一个 process group，也可以创建一个新的 process group。

```c
#include <unistd.h>

int setpgid(pid_t pid, pid_t pgid);  /* 成功返回 0，失败返回 -1 */
```

一个进程只能为它自己或它的子进程设置 process group ID。在大多数的 job-control shells 中，在调用了 `fork` 函数之后，父进程就会调用 `setpgid` 来设置子进程的 process group ID，子进程也会设置自己的 process group ID，好像有点多余？其实不多余，因为父进程和子进程的先后执行顺序是不确定的。

## Sessions

一个 session 是一个或多个进程组的集合。

可以通过一个 shell pipeline 来将多个进程放在同一个进程组里。例如

```bash
proc1 | proc2 &
proc3 | proc4 | proc5
```

一个进程可以调用 `setsid` 函数来 establish 一个新的 session，调用 `getsid` 函数来得到它的 session ID，session ID 就是 session leader 的 PID。

```c
#include <unistd.h>

pid_t setsid(void);

pid_t getsid(pid_t pid);
```

如果调用 `setsid` 函数的进程不是一个 process group leader，则该函数就会创建一个新的 session。

## Controlling Terminal

Sessions 和 process groups 还有一些其他的 characteristics。

- 一个 session 可以有一个 *controlling terminal*。它通常是一个 terminal device 或者是 pseudo terminal device。
- 用来控制 terminal 的 session leader 称为 *controlling process*。
- 一个 session 内的 process groups 可以被分为一个 *foreground process group* 和一个或多个 *background process groups*。
- 如果一个 session 有一个 controlling terminal，则该 session 只有一个 foreground process group，该 session 内的其他 process groups 都是 background process groups。
- 无论什么时候按下 terminal 的 interrupt key（通常是 DELETE 或 Control-C）都会发送 interrupt signal 给 foreground process group 内的所有进程。
- 无论什么时候按下 terminal 的 quit key（通常是 Control-backslash）都会发送 quit signal 给 foreground process group 内的所有进程。
- 如果 terminal interface 检测到了 modem（或 network） disconnect，则 hang-up signal 就会发送给 controlling process（即 session leader）。

## `tcgetpgrp`、`tcsetpgrp` 和 `tcgetsid` Functions

`tcgetpgre` 函数可以获得 foreground process group 的 process group ID。调用 `tcsetpgrp` 函数可以设置 foreground process group 的 process group ID。

```c
#include <unistd.h>

pid_t tcgetpgre(int fd);

pid_t tcsetpgrp(int fd, pid_t pgrpid);
```

大多数的 applications 不会直接调用这两个函数，通常由 job-control shells 调用。

## Job Control

Job control 是在 1980 左右被添加进 BSD 的一个 feature。该 feature 可以让我们从单个 terminal 上开启多个 jobs（groups of processes），并可以控制哪个 jobs 可以访问 terminal，哪个 jobs 是在 background 中运行的。

Job control 可以用过下面 3 个形式来做到：

1. 支持 job control 的 shell。
2. 内核里的 terminal driver 支持 job control。
3. 内核明确了一些 signals 是 job-control signals。

我们可以从一个 shell 中使用 job control，我们可以以 foreground 或 background 的模式启动一个 job。一个 job 只是进程的一个集合，通常是一个 pipeline of processes。例如：

```bash
vi main.c
```

就启动了一个在 foreground 中的 job，该 job 只有一个进程。又如：

```bash
pr *.c | lpr &
make all &
```

就在 background 中启动了 2 个 jobs。哪些由 background jobs 启动的进程都是以 background 的模式运行的。

当启动一个 background job 的时候，shell 会给它一个 job identifier 并打印出一个或多个 process IDs。例如：

```ksh
$ make sleep > Make.out &
[1] 2848

$
[1]  + 2848 done       make sleep > Make.out
```

`make` 的 jib number 就是 1，而 starting process ID 为 2848，当 job 完成的时候，shell 也会告诉我们。

我们也可以用一个特殊的 terminal character 挂起 foreground job：the suspend key（通常是 Control-Z）。输入这个字符会让 terminal driver 发送一个 `SIGTSTP` signal 给 foreground process group 里的所有进程。

The terminal driver 通常有 3 个用来给 foreground process group 发送 signals 的特殊字符：

1. interrupt character，发送 `SIGINT`。
2. quit character，发送 `SIGQUIT`。
3. suspend character。

## Shell Execution of Programs

shells 是如何执行 programs 的呢？这个过程会涉及到 process groups、controlling terminals 和 sessions 的概念。让我们从 `ps` 命令开始。

```bash
$ ps -o pid,ppid,pgid,sid,tpgid,comm
  PID  PPID  PGID   SID TPGID COMMAND
10831 10830 10831 10748 11490 bash
11490 10831 11490 10748 11490 ps
```

书中的例子是 Solaris 上 Bourne shell，因为没有 Solaris，所以就用了 Linux 4.4.0 上的 bash。其中 `TPGID` 是用来控制 session 的 terminal 的 process group ID。可以看到 `ps` 的 PPID 是 `bash` 的 PID（10831）。

书中有详细的 shell `fork` 它自己并 `exec` 其他 program 的例子。

## Orphaned Process Groups

一个父进程已经终止的进程称为一个孤儿进程，并会由 `init` 进程收养。
