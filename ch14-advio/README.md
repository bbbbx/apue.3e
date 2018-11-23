# Advanced I/O

下面的这些我们都称为 *advanced I/O*：nonblocking I/O、record locking、I/O multiplexing（`select` 和 `poll` 函数）、asynchronous I/O、`readv` 和 `writev` 函数和 memory-mapped I/O（`mmap` 函数）。Interprocess communication 和之后的章节都需要用到这些概念。

## Nonblocking I/O

在 Section 10.5 中，我们说过 system calls 可以被分为两类：the "slow" ones 和其他类。The slow system calls 可能会永远 block 住进程（或线程？）。

和 disk I/O 相关的 system call 并不可以认为是 slow 的，即使 read 或 write 一个 disk file 可能暂时会 block 调用该函数的进程（或线程？）。

Nonblocking I/O 可以让我们做 I/O 操作时永远不被 blocked，例如 `open`、`read` 或 `write`。

对于一个指定的 file descriptor，我们有 2 种方法可以指明 nonblocking I/O。

1. 调用 `open` 函数（Section 3.3）时，指定 `O_NONBLOCK` flag。
2. 对于一个已经 opened 的 file descriptor，可以调用 `fcntl` 函数（Section 3.14）来将 `O_NONBLOCK` file status flag 打开。

## Record Locking

在某些 applications 中（例如一个 database system），该 application 的进程需要确定只有它自己单独地在对一个文件进行写操作。为了提供进程具有这项 capability，某些 commercial UNIX systems 提供了 record locking。

术语 *record locking* 通常用来描述一个进程具有这样的能力，当有一个进程正在读取或修改一个文件的某 portion 时，该进程可以阻止其他进程修改该文件的该 region。“record” 这个词其实是一个 misnomer，在UNIX kernel 里的文件中，根本就没有 records 这个 notation。另一个更好的术语是 *byte-range locking*，表面意思就是 lock 住了一个文件的某个 range（也有可能是整个文件）。

下表是不同系统提供的各种 record locking。

|System          |Advisory|Mandatory|`fcntl`|`lockf`|`flock`|
|:---------------|:-------|:--------|:------|:------|:------|
|SUS             |    ∙   |         |    ∙  |  XSI  |       |
|FreeBSD 8.0     |    ∙   |         |    ∙  |  ∙    |   ∙   |
|Linux 3.2.0     |    ∙   |    ∙    |    ∙  |  ∙    |   ∙   |
|Mac OS X 10.6.8 |    ∙   |         |    ∙  |  ∙    |   ∙   |
|Solaris 10      |    ∙   |    ∙    |    ∙  |  ∙    |   ∙   |

## I/O Multiplexing

当我们从一个 file descriptor 中 read，然后 write 到另一个 file descriptor 中，我们可以在一个 loop 中使用 blocking I/O 做到。例如：

```c
while (n = read(STDIN_FILENO, buf, BUFSIZE) > 0)
  if (write(STDOUT_FILENO, buf, n) ! = n)
    err_sys("write error");
```

但是，如果我们需要从 2 个 file descriptors 中 read 的时候呢？

在单个进程内对 `read` 和 `write` 都使用 nonblocking I/O，这种类型的 loop 称为 *polling*，这种方法浪费了 CPU time。在 multitasking system 上应该避免使用 polling。

另一种方法称为 *asynchronous I/O*，也就是当一个 file descriptor 准备好做 I/O 操作时，我们让 kernel 发送一个 signal 通知我们。这种方法的限制是 signals 的数量远远小于 file descriptors 的数量。

一个更好的方法是使用 *I/O multiplexing*。首先，我们需要构造一个那些我们感兴趣的 file descriptors 的 list，然后调用一个函数，直到该 list 中任意一个 file descriptor 准备好做 I/O 操作时，该函数才返回。`poll`、`pselect` 和 `select` 这三个函数可以让我们做 I/O multiplexing 操作。

## Asynchronous I/O

使用 `select` 和 `poll` 是一种异步的通知。只有我们询问的时候（通过调用 `select` 或 `poll`）系统才会告诉我们。如在 Chapter 10 中所说，signals 提供了一种异步方式的通知，用来告诉我们发生了某事。

### POSIX Asynchronous I/O

The POSIX asynchronous I/O interfaces 对所有类型的文件进行 asynchronous I/O 操作都提供一个一致的接口。

The asynchronous I/O interfaces 使用 AIO control blocks 来描述 asynchronous I/O operations。`aiocb` structure 定义了 AIO control block。

```c
struct aiocb {
  int             aio_fildes;     /* file descriptor */
  off_t           aio_offset;     /* file offset for I/O */
  volatile void   *aio_buf;       /* buffer for I/O */
  size_t          aio_nbytes;     /* number of bytes to transfer */
  int             aio_reqprio;    /* priority */
  struct sigevent aio_sigevent;   /* signal information */
  int             aio_lio_opcode  /* operation for list I/O */
};
```

在进行 asynchronous I/O 操作之前，需要初始化一个 AIO control block，然后可以调用 `aio_read` 函数来做一次 asynchronous read，或调用 `aio_write` 函数来做一次 asynchronmous write。

```c
#include <aio.h>

int aio_read(struct aiocb *aiocb);

int aio_write(struct aiocb *aiocb);
```

## `readv` and `writev` Functions

`readv` 和 `writev` 函数可以让我们在一个函数内读、写多个非连续的 buffers。这两个操作有时也称为 *scatter read（散布读）* 和 *gather write（聚焦写）*。

```c
#include <sys/uio.h>

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

## `readn` and `writen` Functions

Pipes、FIFOs 和某些 devices（尤其是 terminals 和 networks）具有下列 2 个 properties：

1. 一次 `read` 操作所返回的数据可以少于所要求的数据，即使还没有遇到 end of file 也是这样。
2. 一次 `write` 操作的返回值也可能会少于指定的输出字节数。

通常，在读、写一个 pipe、network device 或 terminal 时，我们可以使用 `readn` 和 `writen` 函数来读取和写入 data 的 N 个 bytes，并处理返回值小于请求值 N 时的情况。实际上，这两个函数只是只是尽可能地调用 N 次 `read` 或 `write` 函数而已。

```c
#include "apue.h"

ssize_t readn(int fd, void *buf, size_t nbytes);

ssize_t writen(int fd, void *buf, size_t nbytes);
```

这两个函数不是任何 standard 的一 part，定义这两个函数是因为后面要用到。

## Memory-Mapped I/O

Memory-mapped I/O 可以让我们将 disk 上的一个 file 映射到 memory 上的一个 buffer 中。

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t len, int prot, int flag, int fd, off_t off);

int mprotect(void *addr, size_t len, int prot);

int msync(void *addr, size_t len, int flags);

int munmap(void *addr, size_t len);
```
