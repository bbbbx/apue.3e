# Interprocess Communication

进程之间，除了可以通过 passing open file 交换信息之外，还有其他使进程通信的方法：interprocess communication（IPC）。

下表示各种类型的 IPC 及各系统对它支持情况。

|IPC type                | SUS | FreeBSD 8.0 | Linux 3.2.0 | Mac OS X 10.6.8 | Solaris 10 |
|:-----------------------|:----|:------------|:------------|:----------------|:-----------|
|half-duplex pipes       | ∙   |   (full)    | ∙           | ∙               | (full)     |
|FIFOs                   | ∙   |    ∙        | ∙           | ∙               | ∙          |
|full-duplex pipes       | allowed | ∙, UDS  | UDS         | UDS             | ∙, UDS     |
|named full-duplex pipes | obsolescent | UDS | UDS         | UDS             | UDS        |
|XSI message queues      | XSI | ∙           | ∙           | ∙               | ∙          |
|XSI semaphores          | XSI | ∙           | ∙           | ∙               | ∙          |
|XSI shared memory       | XSI | ∙           | ∙           | ∙               | ∙          |
|message queues (real-time) | MSG option | ∙ | ∙           |                 | ∙          |
|semaphores                 | ∙ | ∙          | ∙           | ∙               | ∙          |
|shared memory (real-time)  | SHM option | ∙ | ∙           | ∙               | ∙          |
|sockets                    | ∙ |    ∙       | ∙           | ∙               | ∙          |
|STREAMS                    | obsolescent |  |             |                 | ∙          |

其中 “UDS” 表示的是 UNIX domain sockets（Section 17.2），表明该项 feature 由 UDS 来支持。

我们将 IPC 的 discussion 分为 3 个 chapters。本章讨论的是典型的 IPC：pipes、FIFOs、message queues、semaphores 和 shared memory。下一章讨论使用 sockets mechanism 来实现 network IPC。在 Chapter 17 中，我们讨论 IPC 的 advanced features。

## Pipes

Pipes 是 UNIX System 中最老的 IPC 了。

可以通过 `pipe` 函数创建管道。

```c
#include <unistd.h>

int pipe(int fd[2]);
```

其中，`fd[0]` 是用来 reading 的，而 `fd[1]` 是用来 writing 的。`fd[1]` 的 output 是 `fd[0]` 的 input。

## `popen` 和 `pclose`

一个常见的操作就是创建一个 pipe，连接到另一个进程，去 read 另一个进程的 output 或向它的 input 发送点东西，standard I/O library 提供的 `popen` 和 `pclose` 函数就可以做到这一点。

```c
#include <stdlib.h>

FILE *popen(const char *cmdstr, const char *type);

int pclose(FILE *fp);
```

`popen` 先执行 `fork`，然后再调用 exec 执行 *cmdstring*，并返回一个 standard I/O file pointer。如果 *type* 为 `"r"`，则该 file pointer 连接到 *cmdstring* 的 standard output。如果 *type* 为 `"w"`，则该 file pointer 连接到 *cmdstring* 的 standard input。

该函数与 `fopen` 类似。

## Coprocesses

一个 UNIX 系统的 filter 是一个这样的 program，它从 standard input 中 reads，然后 writes 到 standard output 中去。Filters 通常在 shell pipelines 中被 connected linearly。当 generates filter 的 input 和 reads filter 的 output 的都是同一个 program 的时候，该 filter 就成为了一个 *coprocess（协同进程）*。

## FIFOs

FIFOs 有时也被称为 named pipes。Unnamed pipes 只能在 related processes 中使用，即这两个进程共同的祖先进程创建了该 pipe。但是，通过 FIFOs，unrelated processes 也可以交换数据。

FIFO 是一种文件。使用 `stat` structure 中的 `st_mode` memeber 可以知道一个文件是否是 FIFO。

创建一个 FIFO 与创建一个文件类似。

```c
#include <sys/stat.h>

int mkfifo(const char *path, mode_t mode);

int mkfifoat(int fd, const char *path, mode_t mode);
```

## XSI IPC

有 3 种类型的 IPC 我们称为 XSI IPC：message queue、semaphores 和 shared memory，它们之间有很多相似的地方。

### Identifiers and Keys

内核中的每个 *IPC structure*（message queue、semaphore 或 shared memory segment）都被一个非负整数 *identifier* 所引用。

identifier 是一个 IPC object 的 internal name，而 *key* 是一个 IPC object 的 external name。

### Permission Structure

每个 IPC structure 内都有一个 `ipc_perm` structure。该结构定义了 permissions 和 owner 和其他的信息。

```c
struct ipc_perm {
  uid_t  uid;    /* owner’s effective user ID */
  gid_t  gid;    /* owner’s effective group ID */
  uid_t  cuid;   /* creator’s effective user ID */
  gid_t  cgid;   /* creator’s effective group ID */
  mode_t mode;   /* access modes */
};
```

### Configuration Limits

在 FreeBSD 8.0、Linux 3.2.0 和 Mac OS X 10.6.8 中，可以使用 `sysctl` 命令来查看或修改内核的配置参数。

在 Linux 中，可以运行 `ipcs -l` 来查看和 IPC 相关的 limit。

### Advantages and Disadvantages

XSI IPC 的问题是，IPC structures 没有 reference count，它是 systemwide 的。例如，如果我们创建了一个 message queue，并将一些 messages 放到该 queue 中，然后 terminate 该进程，则该 message queue 和它的 contents 并不会被删除。

XSI IPC 的另一个问题就是，这些 IPC structures 在 file system 中并不由它们的 names 来标识。为此，增加了一打的系统调用(`msgget`、`semop`、`shmat` 等等）来获得这些 IPC objects。我们也不能通过 `ls` 命令来查看 IPC objects，也不能通过 `rm` 来移除它们，而是需要使用 `ipcs` 和 `ipcrm` 命令。

## Message Queue

一个 message queue 是 messages 的一个 linked list，该 list 存储在 kernel 中，并用一个 message queue identifier 来标识。

```c
#include <sys/msg.h>
```

## Semaphores（信号量）

一个 semaphore 是一个 counter，用来给多个进程提供对 shared data object 的访问权限。

一个进程为了获取一个 shared resource，需要做以下步骤：

1. 检查控制该 resource 的 semaphore。
2. 如果该 semaphore 的值是正的，则该进程可以使用该 resource，并将该 semaphore 减一，表明它使用了该 resource 的 1 个 unit。
3. 否则，如果该 semaphore 的值为 0，则该进程就去 sleep，直到该 semaphore 的值比 0 大。但该进程 wakes up 的时候，它就返回 step 1。

当有一个进程不再使用该 shared resource 时，则控制该 shared resource 的 semaphore 的值加 1，并 wake up 其他正在等待该 semaphore 的进程。

```c
#include <sys/sem.h>
```

## Shared Memory

Shared memory 可以让多个进程共享 memory 指定的一个 region。这是最快的一种 IPC，因为 data 不需要在 client 和 server 之间被 copied。

```c
#include <sys/shm.h>
```

## POSIX Semaphores

The POSIX semaphore interfaces 意在解决 the XSI semaphore interfaces 的某些缺陷。

```c
#include <semaphore.h>
```
