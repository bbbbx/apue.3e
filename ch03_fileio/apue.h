/**
 * 我们自己的 header，包含了所有的 standard system headers。
 */
#ifndef _APUE_H
#define _APUE_H

#define _POSIX_C_SOURCE 200809L

#if defined(SOLARIS)             /* Solaris 10 */
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 700
#endif

#include <sys/types.h>           /* 有些系统还需要这个 */
#include <sys/stat.h>
#include <sys/termios.h>         /* for winsize */

#if defined(MACOS) || !defined(TIOCGWINSZ)
#include <sys/ioctl.h>
#endif

#include <stdio.h>               /* for convenience */
#include <stdlib.h>              /* for convenience */
#include <stddef.h>              /* for offset */
#include <string.h>              /* for convenience */
#include <unistd.h>              /* for convenience */
#include <signal.h>              /* for SIG_ERR */

#define MAXLINE 4096             /* max line length */

/**
 * 新文件的默认 file access permissions
 */
#define FILE_MODE    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/**
 * 新目录的默认 permissions
 */
#define DIR_MODE     (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

typedef void Sigfunc(int);        /* for signal handlers */

#define min(a,b)      ((a) < (b) ? (a) : (b))
#define max(a,b)      ((a) > (b) ? (a) : (b))

/**
 * 我们自己的函数的 prototypes
 */
char *path_alloc(size_t *);
long open_max(void);

int set_cloexec(int);
void clr_fl(int, int);
void set_fl(int, int);

void pr_exit(int);

void pr_mask(const char *);
Sigfunc *signal_intr(int, Sigfunc *);

void daemonize(const char *);

void sleep_us(unsigned int);
ssize_t readn(int, void *, size_t);
ssize_t writen(int, const void *, size_t);

int fd_pipe(int *);
int recv_fd(int, ssize_t (*func) (int, const void *, size_t));   /* Figure 17.14 */

void err_msg(const char *, ...);
void err_quit(const char *, ...) __attribute__((noreturn));
void err_exit(int, const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...) __attribute__((noreturn));

#endif
