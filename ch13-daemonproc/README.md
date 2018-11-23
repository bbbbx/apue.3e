# Daemon Processes

Daemons 是一种运行时间比较长的进程。通常在系统 bootstrapped 之后就运行，且当系统 shut down 的时候才终止。Daemon 没有一个 controlling terminal，那 daemon 在出错的时候是如何 report error conditions 的呢？。

<small>有关 *daemon* 这一术语是如何被应用到计算机系统中的历史背景，可以看 [Raymond 1996 *The New Hacker's Dictionary, Third Edition.* MIT Press, Cambridge, MA.]，这本书中说明了大量的 computer hacker 的术语</small>

## Daemon Characteristics

运行命令：

```bash
ps -axj
```

可在 BSD-base 的系统中看到有关我们本次 discussion 多需的信息。`-a` 会显示其他用户拥有的进程的 status，`-x` 会显示没有 controlling terminal 的进程，`-j` 会显示和 job 相关的信息：session ID、process group ID、controlling terminal 和 terminal process group ID。

System V–based 的系统可以运行 `ps -efj`。

Linux 4.4.0 运行结果的一部分：

```bash
 PPID   PID  PGID   SID TTY      TPGID STAT   UID   TIME COMMAND
    0     1     1     1 ?           -1 Ss       0   2:22 /lib/systemd/systemd --system --deserialize 28
    0     2     0     0 ?           -1 S        0   0:00 [kthreadd]
    2     3     0     0 ?           -1 S        0   4:13 [ksoftirqd/0]
    2     5     0     0 ?           -1 S<       0   0:00 [kworker/0:0H]
    2     7     0     0 ?           -1 S        0   6:56 [rcu_sched]
    2     8     0     0 ?           -1 S        0   0:00 [rcu_bh]
    2     9     0     0 ?           -1 S        0   0:00 [migration/0]
    2    10     0     0 ?           -1 S        0   1:34 [watchdog/0]
    2    28     0     0 ?           -1 S        0   1:44 [kswapd0]
    1   547   547   547 ?           -1 Ss       0   0:00 /usr/sbin/inetd -i
    1  1342  1342  1342 ?           -1 Ssl    104   1:16 /usr/sbin/rsyslogd -n
    1  1336  1336  1336 ?           -1 Ss       0   0:27 /usr/sbin/cron -f
    1  1357  1357  1357 ?           -1 Ss       1   0:00 /usr/sbin/atd -f
    1 11426 11426 11426 ?           -1 Ss       0   0:05 /usr/sbin/sshd -D
    1 17604 17604 17604 ?           -1 Ss       0   9:55 /sbin/rpcbind -f -w
    1 17804 17804 17804 ?           -1 Ss       0   0:00 /usr/sbin/rpc.idmapd
    1 17809 17809 17809 ?           -1 Ss       0   0:00 /usr/sbin/rpc.mountd
    2 27367     0     0 ?           -1 S<       0   0:00 [rpciod]
```

其中，PPID 为 0 的进程通常是 kernel process，它是作为 system bootstrap procedure 的一部分来运行的，例如 `kthreadd`。不过，`init`（之前是 `init`，Linux 4.4.0 中为 `systemd` 进程，即 PID 为 1 的进程）进程是个例外，它是一个由 kernel 在 boot time 启动的 user-level command。

在上面的例子中，出现在方括号内的都是 kernel daemons。该版本的 Linux 使用了一个特殊的 kernel process（`kthreadd`）来创建其他的 kernel processes，所以，`kthreadd` 是其他 kernel daemons 的父进程。

还有，在 Linux 中，`kswapd` daemon 是 pageout daemon，`rpcbind` daemon 支持将 RPC program numbers 映射成 network port numbers 的服务。`rsyslogd` daemon 可以让其他 program 打印出 system messages，`inetd` 监听 system 的 network interfaces，将 incoming requests 发到不同的 network servers。

`cron` daemon 会在指定的时间点执行指定的 commands，`atd` daemon 和 `cron` 类似，它可以让用户在指定的时间点执行指定的 jobs。`sshd` daemon 提供 secure remote login 和 execution facilities。

大多数的 daemons 都是以 superuser (root) privileges 来运行的。没有一个 daemon 有 controlling terminal：即 the terminal name 被设置成了 question mask（问号 ?）。而 user-level 的 daemons 的父进程是 `init`（即现在的 `systemd`） 进程。

## Coding Rules

为了编写一个没有 interactions 的 daemon，我们需要遵循一些 basic rules。`daemonize` 函数实现了这些规则。

1. 调用 `umask` 函数将 file mode creation mask 设置为一个确定的值（通常是 0）。
2. 调用 `fork` 函数之后，父进程需要调用 `exit` 函数。
3. 调用 `setsid` 函数去创建一个新的 session。
4. 将 current working directory 改为 the root directory，当然，也可以是其他目录。例如，一个 line printer spooling daemon 可能会将它的 working directory 改为它的 spool directory。
5. 关闭不需要使用的 file descriptors。
6. 某些 daemons 可能还会将 file descriptors 0、1 和 2 指向 `/dev/null`。

## Error Logging

Daemon 不能将 error message 写到 standard error 中去，因为它没有  controlling terminal。我们也不想每个 daemon 都将它自己的 error message 写到一个单独的文件中。如果需要记得哪个 daemon 将 error message 写到哪个 log file 中，并定期检查这些 log files，那一定很痛苦。所以我们需要一个 central daemon error-logging facility。

自 4.2BSD 以来，BSD `syslog` facility 被广泛使用。大多数的 daemons 都是使用这个 facility。它的结构如下：

```txt
                                + - - - - - - - - -+
                                ' kernel:          '
                                '                  '
                                ' +--------------+ '     +---------+
    TCP/IP network    --------> ' | UDP port 514 | ' --> | syslogd | <+
                                ' +--------------+ '     +---------+  |
                                '                  '       ^          |
                                '                  '       |          |
                                '                  '       |          |
  +-----------------+  syslog   ' +--------------+ '       |          |
  |  user process   | --------> ' |   /dev/log   | ' ------+          |
  +-----------------+           ' +--------------+ '                  |
+ - - - - - - - - - - - - - - -                    '                  |
'                                                  '                  |
' +-----------------+  log        +--------------+ '                  |
' | kernel routines | ----------> |  /dev/klog   | ' -----------------+
' +-----------------+             +--------------+ '
'                                                  '
+ - - - - - - - - - - - - - - - - - - - - - - - - -+
```

<small>上图使用 Graph-Easy 制作，命令如下：

```bash
$ graph-easy <<< "[user process] - syslog -> [/dev/log]
[TCP/IP network]{border:none} -> [UDP port 514]
[kernel routines] - log -> [/dev/klog]
[/dev/log], [UDP port 514], [/dev/klog] -> [syslogd]
(kernel: [/dev/log], [UDP port 514], [kernel routines], [/dev/klog], [/dev/log])"
```
</small>

其中，UDP port 514、/dev/log 、/dev/klog 和 kernel routines 这四个部件处于系统的 kernel 里。

有 3 种方法产生 log messages：

1. Kernel routines 可以调用 `log` 函数来产生 log messages。这些 messages 可以被其他的 user process 通过调用 `opens` 和 `reads` 函数指向 `/dev/klog` device 来获得。我们写的 program 都不是 kernel routines，所有这个函数我们不会使用。
2. 大多数的 user processes（即 daemons）可以调用 `syslog` 函数来产生 log messages。这些 messages 会被发送到 the UNIX domain datagram socket `/dev/log`。
3. 在该 host 或其他通过 TCP/IP network 连接该 host 的 hosts 上的 user process 也可以发送 log messages 到 UDP port 514 上。注意，`syslog` 函数不会生成这些 UDP datagrams。

通常来说，`syslogd` daemon 会读取这 3 种 log messages 的格式。在启动的时候，该 daemon 通常会读取 `/etc/syslog.conf` 配置文件，该文件决定了不同类型的 messages 应该发到哪里去。

## Single-Instance Daemons

有些 daemons 是只能同时运行一个 copy of the daemon。

The file-locking 和 record-locking mechanism（Section 14.3）确保了只有一个 copy of a daemon 在运行。

## Daemon Conventions

在 UNIX 系统中，daemons 遵循着下列的惯例：

- 如果 daemon 使用了一个 lock file，则该文件通常存储在 `/var/run` 目录下。lock file 的文件名通常是 *name*.pid，其中，*name* 是该 daemon 的名字。
- 如果该 daemon 支持 configuration options，则它们通常存储在 `/etc` 目录下。这个 configuration file 的文件名通常是 *name*.conf，其中，*name* 是该 daemon 的名字。
- 因为 daemon 可以从 command line 启动，所以，它们通常都是由 one of the system initialization scripts（`/etc/rc*` 或 `/etc/init.d/*`）。
- 如果该 daemon 有一个 configuration file，那么该 daemon 会在启动的时候读取这个配置文件，但在启动之后通常都不会再次读取该配置文件了。所以，如果修改了配置文件，那就需要重启该 daemon。

## Client–Server Model

一个 daemon process 通常被当作一个 server process。

一个 *server* 就是一个进程，该进程等待一个 *client* 来和它 contact。

通常，server 都会 `fork` 一个子进程，然后该子进程再 `exec` 另一个 program 来给 client 提供服务。
