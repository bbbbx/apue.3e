# System Date Files and Information

本章包含 system identification functions 和 the time 和 date functions。

## Password File

UNIX 系统的 password file（POSIX.1 称其为 user database）包含了下列的字段，这些字段在 `<pwd.h>` 的 `passwd` structure 中。

|Description            |struct `passwd` member|
|:----------------------|:---------------------|
|user name                     |char   *pw_name   |
|加密后的 password              |char   *pw_passwd |
|user ID                       |uid_t  *pw_uid    |
|group ID                      |gid_t  *pw_gid    |
|comment field                 |char   *pw_gecos  |
|初始的 working directory       |char   *pw_dir    |
|初始的 shell                   |char   *pw_shell  |
|user access class             |char   *pw_class  |
|next time to change password  |time_t pw_change  |
|account expiration time       |time_t pw_expire  |

由于历史原因，password file 存在 `/etc/passwd` 中，且是一个 ASCII 文件。字段之间用冒号隔开。

使用 `finger` 命令可以打印出有关账户的信息：

```bash
$ finger -p venus
Login: venus          			Name: Venus
Directory: /Users/venus             	Shell: /bin/zsh
On since Sun Nov 18 12:29 (CST) on console, idle 1:36 (messages off)
On since Sun Nov 18 12:33 (CST) on ttys000
On since Sun Nov 18 13:37 (CST) on ttys002 (messages off)
New mail received Tue Oct 30 19:19 2018 (CST)
     Unread since Mon Oct 29 22:34 2018 (CST)
```

某些系统可以使用 `vipw` 命令来编辑 password file。

## Shadow Passwords

为了让别人难以获得加密后的密码，系统将加密后的密码存放在另一个文件中，称为 *shadow password file*。该文件至少包含 user name 和 encrypted password。

只有少数的 programs 需要访问 encrypted passwords，如 `login` 和 `passwd`，这些 programs 通常都是 set-user-ID root。

在 Linux 3.2.0 和 Solaris 10 中，shadow password file 是 `/etc/shadow` 文件，它的每一项在 `<shadow.h>` 文件中的 `spwd` structure 中，和 password file 类似。

在 FreeBSD 8.0 和 Mac OS 10.6.8 中，没有 shadow file structure。额外的账户信息被放在 password file 中（图 6.1）。

## Group File

Group file 在 `/etc/group` 文件中，文件中的字段包含在 `<grp.g>` 中所定义的 `group` structure 中。

## Supplementary Group IDs

4.2BSD 引入了 附属组 ID（supplementary group ID）的概念。即一个账户可以属于多个组。

## Other Data Files

除了 password file 和 group file 这两个系统的 data file 之外，还有其他的 data files，例如 BSD network software 有一个记录所提供的服务的 `/etc/services`、记录协议信息的 `/etc/protocols` 和记录网络信息的 `/etc/networks`。访问这些 data files 和上述的 password file 和 group file 类似。

## Login Accounting

大多数的 UNIX 系统都提供了下面两个 data file：`utmp` 文件，它记录了目前已经登录的所有用户，`wtmp` 文件，它记录了所有的登录和登出。

```c
struct utmp {
  char   ut_line[8];   /* tty line: "ttyh0", "ttyd0", "ttyp0" */
  char   ut_name[8];   /* login name */
  long   ut_time;      /* seconds since Epoch */
};
```

例如，登录的时候，`login` program 需要写 `utmp` 文件和 `wtmp` 文件，而登出的时候，`init` process 需要写 `wtmp` 文件和清除 `utmp` 文件中对应的记录。而`who` program 需要读取 `utmp` 文件，`last` command 需要读取 `wtmp` 文件。

在 FreeBSD 8.0 和 Linux 3.2.0 中，这两个文件的路径名是 `/var/run/utmp` 和 `/var/log/wtmp`。

## System Identification

POSIX.1 定义了 `uname` 函数，它返回当前主机与操作系统的信息。

```c
#include <sys/utsname.h>

int uname(struct utsname *name);

struct utsname {
  char sysname[];   /* 操作系统的 name */
  char nodename[];  /* 该 node 的 name */
  char release[];   /* 该操作系统当前的 release */
  char version[];   /* 该 release 的当前 version */
  char machine[];   /* hardware type 的 name */
};
```

通常可以使用 `uname` 命令可以打印出 `utsname` structure 中的信息。

`hostname` 命令可以用来获取和设置主机名。

## Time and Date Routines

UNIX 内核提供的 time service 是从 1970 年 1 月 1 日 00:00:00（Coordinated Universal Time, UTC）到现在的秒数。这种秒数以数据结构 `time_t` 来表示，我们称它们为 *calendar time*。这些 calendar time 代表着 time 和 date。

而 `time` 函数返回当前的 time 和 date。

```c
#include <time.h>

time_t time(time_t *calptr);
```

如果参数不是 `NULL`，则 time 值也存到 `calptr` 指向的地方中。

```c
#include <time.h>

struct tm * gmtime(const time_t *calptr);

struct tm * localtime(const time_t *calptr);
```

`localtime` 将 calendar time 转为本地时间，而 `gmtime` 将 calendar time 转为 UTC 的分解结构。

函数 `strftime` 类似于 `printf`。它非常复杂，可以通过多个参数来定制生成的时间字符串。

```c
#include <time.h>

size_t strftime(char *restrict buf, size_t maxsize, const char *restrict format, const struct tm *restrict tmptr);

size_t strftime_l(char *restrict buf, size_t maxsize, const char *restrict format, const struct tm *restrict tmptr, locale_t locale);
```

`strftime_l` 允许指定的 locale 作为参数。
