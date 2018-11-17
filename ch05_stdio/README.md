# Standard I/O Library

## Streams and FILE Objects

在第 3 章中，所有的 I/O 操作都是围绕着 file descriptor 的，而在 standard I/O library 中，它们的操作是围绕着 *stream* 来的。

## Formatted I/O

### 格式化输出

```c
#include <stdio.h>

/* 写到标准输入，成功则返回输出字符的个数，失败则返回负数 */
int printf(const char *restrict format, ...);

/*  写到指定的 stream，成功则返回输出字符的个数，失败则返回负数 */
int fprintf(FILE *restrict fp, const char *restrict format, ...);

/* 写到指定的 file descriptor，成功则返回输出字符的个数，失败则返回负数 */
int dprintf(int fd, const char *restrict format, ...);

/* 写到数组 `buf` 中，可能回造成 `buf` 指定的缓存区溢出  */
int sprintf(char *restrict buf, const char *restrict format, ...);

/* 需要指定缓冲区长度 `n` */
int snprintf(char *restrict buf, size_t n, const char *restrict format, ...)
```

conversion specification 控制了如何 encode 剩余的 arguments 和最终的 display。一个 conversion specification 以百分号（%）开始。一个 conversion specification 有四个可选的 components，即下面的方括号内的：

```plain
%[flags][fldwidth][precision][lenmodifier]convtype
```

flags 如下：

|Flag|Description|
|:---|:----------|
|'   |撇号，将整数格式化为具有千分位字符|
|-   |将输出字段左对齐               |
|+   |将输出字段右对齐               |
| （space)|如果没有符号，则加上一个 space 作为前缀|
|#        |转换为另一种格式（例如给十六进制加上 `0x` 前缀）|
|0        |使用 0 作为前缀，而非使用 space 作为前缀      |

fldwidth 定义的是字段的最小宽度。

对于 integer conversions 来说，precision 是最小的 number of digits to appear；对于 floating-point conversions 来说，precision 是小数部分的最小的 number of digits to appear。
The precision 以一个点（.）和一个可选的非负十进制整数组成。

lenmodifier 表明了 argument 的 size。

|Length modifier|Description|
|:--------------|:----------|
|hh             |像对应的 argument 按 signed or unsigned char 类型输出|
|h              |像对应的 argument 按 signed or unsigned short 类型输出|
|l              |signed or unsigned long or wide character|
|ll             |signed or unsigned long long|
|j              |intmax_t or uintmax_t|
|z              |size_t|
|t              |ptrdiff_t|
|L              |long double|

convtype 是必须要的。

|Conversion type|Description|
|:--------------|:----------|
|d, i           |有符号的十进制的|
|o              |无符号的八进制的|
|u              |无符号的十进制的|
|x, X           |无符号的十六进制的|
|f, F           |double floating-point number|
|e, E           |指数形式的 double floating-point number|
|g, G           |根据转换后的值解释为 f、F、e 或 E|
|a, A           |十六进制的指数形式的 double floating-point number|
|c              |character（如果带有 l length modifier，则为 wide character）|
|s              |字符串（如果带有 l length modifier，则为 wide character string）|
|p              |一个指向 `void` 的 pointer|
|n              |一个指向 signed integer 的 pointer，该 signed integer 是到目前为止被写入的字符的个数|
|%              |一个 % 字符|
|C              |wide character（XSI option，等效于 `lc`）|
|S              |wide character string（XSI option，等效于 `ls`）|

### 格式化输入

```c
#include <stdio.h>

int scanf(const char *restrict format, ...);

int fscanf(FILE *restrict fp, const char *restrict format, ...);

int sscanf(const char *restrict buf, const char *restrict format, ...);
```

在 `format` 参数之后的 arguments 是变量的地址。

## Implementation Details

每个 standard I/O stream 都有一个与其相关的 file descriptor，可以通过调用 `fileno` 函数来获取一个 stream 的 descriptor。

```c
#include <stdio.h>

int fileno(FILE *fp);
```
