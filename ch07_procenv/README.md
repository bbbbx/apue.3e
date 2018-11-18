# Process Environment

## Process Termination

### Exit Function

```c
#include <stdlib.h>

void exit(int status);

void _Exit(int status);

#include <unistd.h>

void _exit(int status);
```

将 `main` 声明的返回值为 `void` 时，调用 `exit` 函数退出；将 `main` 声明的返回值为 `int` 时，执行 `return` 语句退出。

### `atexit` Function

一个进程可以 register 32 个函数，这些函数将由 `exit` 自动调用，称这些函数为 *exit handler*。

```c
#include <stdlib.h>

int atexit(void (*func)(void));
```

其中，`atexit` 函数的参数是一个函数地址。`exit` 函数调用这些函数的顺序与 register 它们的顺序相反。

## Command-Line Arguments

`int argc` 和 `char *argv[]`。

## Environment List

每个进程都会被传进一个 *environment list*。就像 the argument list 那样，the environment list 也是一个字符指针数组。全局变量 `environ` 包含了该指针数组的地址：

```c
extern char **environ;
```

在历史上，大多数的 UNIX 系统支持 `main` 函数带 3 个参数，其中第三个参数就是 the environment list 的地址：

```c
int main(int argc, char *argv[], char *envp[]);
```

因为 ISO C 规定 `main` 函数只有两个参数，所以 POSIX.1 也规定应使用 `environ`  变量，而不使用第 3 个参数。通常用 `getenv` 和 `putenv` 函数（7.9 节）来访问指定的环境变量，而不是直接使用 `environ`。但是，如果要遍历整个 environment，则必须使用 `environ` 指针。

## Memory Layout of a C Program

一个 C program 由下列几部分组成：

- Text segment，包含了 CPU 执行的机器指令。通常是只读的，为了防止其他的 program 修改该 program 的指令。
- Initialized data segment，通常称为 data segment，其包含已经初始化的变量，例如：
    ```c
    int     maxcount = 99;
    ```
- Uninitialized data segment，通常称为 the "bss" segment，该名字来源于早期汇编语言的一个操作符 “block started by symbol”。在该 program 开始执行之前，内核将该 segment 内的 data 初始化为算术 0 或 null pointer。例如：
    ```c
    long    sum[1000];
    ```
- Stack，用来存放 automatic variables 以及每次函数调用都需要保留的信息。
- Heap，通常用来动态分配内存。由于历史原因，heap 位于 BSS 和 stack 之间。

对于在一个 32-bit Intel x86 processor 上的 Linux 而言，它的 text segment 始于 `0x08048000`，而 stack 的 bottom 从 `0xC0000000` 以下开始（在这种结构中，stack 从高地址向低地址增加）。堆顶和栈顶之间未使用的 virtual address space 非常的大。

可以使用 `size` 命令打印 text、data 和 bss segment 的大小（byte单位）。

```bash
$ size /bin/sh
   text	   data	    bss	    dec	    hex	filename
 143301	   4792	  11312	 159405	  26ead	/bin/sh
```

第 4 列和第 5 列分别是以十进制和十六进制形式的这 3 个 segment 的总长。

## Shared Libraries

`cc` 命令和 `ld` 命令。

## Memory Allocation

ISO C 说明了三个用于 memory allocation 的函数：

```c
#include <stdlib.h>

/* 初始值不确定，出错则返回 NULL */
void *malloc(size_t size);

/* 初始值为 0 */
void *calloc(size_t nobj, size_t size);

/* 增加或减少之前的 allocated area 的大小，初始值不确定 */
void *realloc(void *ptr, size_t newsize);


void free(void *ptr);
```

### 代替的 Memory Allocators

**libmalloc**

基于 SVR 4 的 UNIX 系统包含了 `libmalloc` 库，例如 Solaris。

**vmalloc**

**quick-fit**

**jemalloc**

FreeBSD 8.0 中默认的 memory allocator。

**TCMalloc**

**alloca Function**

和 `malloc` 一样，不过它不是从 heap 中分配内存的，而是从当前的函数中的 stack frame 中。因此，函数返回时会自动释放它所使用的 stack frame。本书所讨论的 4 个系统都支持 `alloca` 函数。

## 环境变量

```c
#include <stdlib.h>

char *getenv(const char *name);

/* rewrite 表示是否要覆盖之前的值 */
int setenv(const char *name, const char *value, int rewrite);

/* str 的形式需要为 name=value */
int putenv(char *str);

int unsetenv(const char *name);
```

返回的指针指向 `name=value` 中的 value 字符串，未找到则返回 `NULL`。

命令行参数和环境变量通常放在进程的 memory space 的顶部，即 stack 的上面。

## `setjmp` and `longjmp` Functions

在 C 中，我们不能 goto 到另一个函数中的 label 中，不过我们可以使用 `setjmp` 和 `longjmp` 函数做到这一点。

```c
#include <setjmp.h>

int setjmp(jmp_buf env);

void longjmp(jmp_buf env, int val);
```

## `getrlimit` and `setrlimit` Functions

每个进程都有一个资源限制的 set，其中的一些限制可以通过 `getrlimit` 函数和 `setrlimit` 函数获取和修改。

```c
#include <sys/resource.h>

/* 成功返回 0，出错返回 -1 */
int getrlimit(int resource, struct rlimit *rlptr);

int setrlimit(int resource, const struct rlimit *rlptr);
```

其中 `rlimit` 结构如下：

```c
struct rlimit {
  rlim_t  rlim_cur;   /* soft limit: 当前的限制 */
  rlim_t  rlim_max;   /* hard limit: rlim_cur 的最大值 */
};
```

常量 `RLIMIT_INFINITY` 指定了一个无限量的限制。
