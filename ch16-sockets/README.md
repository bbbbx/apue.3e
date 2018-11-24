# Network IPC: Sockets

上一章中的 pipes、FIFOs、message queues、semaphores 和 shared memory，这些都是同一台电脑上不同进程间的 IPC。而使用 network IPC interface 的 processes，不管是不是同一台电脑上的进程，它们之间都可以通信。

POSIX.1 声明的 socket API 是基于 4.4BSD socket interface 的，其源自于 1908s 的 4.2BSD。

<small>[Stevens, W. R. 1990. *UNIX Networking Programming.*] 和 [Stevens, W. R., Fenner, B., and Rudoff, A. M. 2004. *UNIX Networking Programming, Volume 1, Third Edition*] 详细地说名了在 UNIX 系统上进行 network programming。</small>

## Socket Descriptors

一个 socket 是一个 communication endpoint 的一个抽象。就像我们使用 file descriptors 来访问 file 一样，applications 使用 socket descriptors 来访问 sockets。在 UNIX 系统中，socket descriptors 被当作 file descriptors 来实现。

可以调用 `socket` 函数来创建一个 socket。

```c
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

`domain` 参数指定了该 communication 的 nature，这些 nature 包括 address 的格式。下表示 POSIX.1 指明的 domains。这些常量都以 `AF_`（*address family*）开头，因为每个 domain 的 address 都有它自己的表示格式。

|Domain       |Description      |
|:------------|:----------------|
|AF_INET      |IPv4 Internet domain|
|AF_INET6     |IPv6 Internet domain (optional in POSIX.1)|
|AF_UNIX      |UNIX domain      |
|AF_UNSPEC    |unspecified      |

`type` 参数指定了 socket 的类型，它进一步地确定了 communication 的 characteristics。下表示 POSIX.1 指明的 socket types。

|Type            |Description      |
|:---------------|:----------------|
|SOCK_DGRAM      | fixed-length, connectionless, unreliable messages |
|SOCK_RAW        | datagram interface to IP (optional in POSIX.1)    |
|SOCK_SEQPACKET  | fixed-length, sequenced, reliable, connection-oriented messages |
|SOCK_STREAM     | sequenced, reliable, bidirectional, connection-oriented byte streams |

`protocol` 参数通常是 0，表示对于指定的 domain 和 socket type 选择它们的 default protocol。例如，对于 `AF_INET` 和 `SOCK_STREAM` 的默认协议是 TCP，对于 `AF_INET` 和 `SOCK_DGRAM` 的默认协议是 UDP。下表列出了 Internet domain sockets 定义的 protocols。

|Protocol            |Description         |
|:---------------|:-----------------------|
|IPPROTO_IP      | IPv4 Internet Protocol |
|IPPROTO_IPV6    | IPv6 Internet Protocol (optional in POSIX.1)    |
|IPPROTO_ICMP    | Internet Control Message Protocol |
|IPPROTO_RAW     | Raw IP packets protocol (optional in POSIX.1) |
|IPPROTO_TCP     | Transmission Control Protocol |
|IPPROTO_UDP     | User Datagram Protocol |

调用 `socket` 和调用 `open` 类似。

在一个 socket 上进行通信是双向的，可以用 `shutdown`  函数来 disable 一个 socket 上的 I/O。

```c
#include <sys/socket.h>

int shutdown(int sockfd, int how);
```

## Addressing

如何标识一个我们想要和它进行通信的进程呢？标识该进程有两个 components，network address 用来标识我想要进行通信的 computer，而 *port number* 代表一种 service，用来帮助我们标识在该 computer 上一个指定的进程。

### Byte Ordering

如果需要通信的进程是在同一台电脑上运行的，则我们并不需要担心 byte ordering。The byte order 是 CPU 架构的一个 characteristic，表明了 bytes 是如何在 larger data types（例如 integers）内排序的。

CPU 架构有 *big-endian* 的和 *little-endian* 的，大部分是 little-endian 的。

而 network protocols 指定了一个 byte ordering，这样各种各样的计算机才不会因为 byte ordering 的不同而不能交换 protocol information。

TCP/IP protocol suite 使用的是 big-endian byte order。

在 TCP/IP 中，addresses 用 network byte order 来表示，所有 applications 有时需要将 addresses 从 CPU 的 byte order 转换为 network byte order。例如，如果需要将一个 address 打印成 human-readable 的格式时，通常就需要转换成 network byte order。

TCP/IP applications 可以使用下列的函数来在它们之间相互转换。

```c
#include <arpa/inet.h>

uint32_t htonl(uint32_t hostint32);  /* 返回 以 network byte order 的 32-bit integer */

uint16_t htons(uint16_t hostint16);

uint32_t ntohl(uint32_t netint32);

uint16_t ntohs(uint16_t netint16);
```

其中，`h` 是 "host" byte order，而 `n` 是 "network" byte order。而 `l` 是 "long"（即 4 个 bytes）integer，而 `s` 是 "short"（即 2 个 byes）integer。
