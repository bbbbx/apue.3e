# Files and Directories

## stat, fstat, fstatat and lstat Functions

```c
#include <sys/stat.h>

int stat(const char *restrict pathname, struct stat *restrict buf);

int fstat(int fd, struct stat *buf);

int lstat(const char *restrict pathname, struct stat *restrict buf);

int fstatat(int fd, const char *restrict pathname, struct stat *restrict buf, int flag);
/* 这四个函数的返回值：OK 则返回 0，出错则返回 -1 */
```

其中，参数 `buf` 是指向一个 structure 的指针，函数会填充该 structure structure 的定义会随 implementations 的不同而不同，不过它的定义类似这样：

```c
struct stat {
  mode_t             st_mode;     /* 文件类型和 mode（permissions） */
  ino_t              st_ino;      /* i-node number */
  dev_t              st_dev;      /* device number */
  dev_t              st_rdev;     /* 特定文件的 device number */
  nlink_t            st_nlink;    /* number of links */
  uid_t              st_uid;      /* uesr ID of owner */
  gid_t              st_gid;      /* group ID of owner */
  off_t              st_size;     /* size in bytes，对 regular file 有效 */
  struct timespec    st_atime;    /* last access 的时间 */
  struct timespec    st_mtime;    /* last modification 的时间 */
  struct timespec    st_ctime;    /* last file status change 的时间  */
  blksize_t          st_blksize;  /* best I/O block size */
  blkcnt_t           st_blocks;   /* number of disk blocks allocated */
};
```

### Ownership of New Files and Directories

一个新的文件的 user ID 会被设置为该 process 的 effective user ID。

### File Systems

有很多中文件系统：

- 传统的、基于 BSD 的 UNIX 文件系统（称为 UFS）
- 读、写 DOS-formatted diskettes 的文件系统（称为 PCFS）
- 读取 CD 的文件系统（称为 HSFS）

大多数的 UNIX 文件系统都是大小写敏感的。在 Mac OS X 上，HFS 文件系统是大小写保留的（case-preserving）和大小写不敏感的。

我们可以把一个磁盘（disk）分成一个或多个分区（partition）。每个分区都可以包含一个文件系统。

每个分区由一个 boot block、一个 super block 和多个 cylinder group 组成。

每个 cylinder group  由一个 super block copy、一个 config info、一个 i-node map、一个 block bitmap、一个 i-nodes 和多个 data blocks 组成。

一个 i-nodes 由多个 i-node 组成。The i-node 是固定长度的，它包含了一个 file 的大多数 information。

- 磁盘
    - 分区
    - 分区
        - boot block
        - super block
        - cylinder group 0
        - cylinder group 1
            - super block copy
            - cg info
            - i-node map
            - block bitmap
            - i-nodes
                - i-node
                - i-node
                - i-node
                - ...
            - data blocks
                - data block
                - directory block
                    - entry 1
                        - i-node number
                        - filename
                    - entry 2
                    - ...
                    - entry n
                - ...
        - ...
        - cylinder group n
    - 分区

其中同一个一个 i-node 可以指向多个 data block，多个 directory block 中的 i-node number 可以指向同一个 i-node。上面这种格式的文件系统与传统的 UNIX 文件系统很相似，[Bach, M. J. 1986 *The Design of the UNIX Operating System*] 的第 4 章详细地描述了传统的 UNIX 文件系统。