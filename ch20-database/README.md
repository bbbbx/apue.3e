# A Database Library

## History

UNIX 系统中最火的一个 database functions library 是 `dbm`(3) library（dbm 是 DataBase Manager 的 three-letter acronym）。该 library 最早由 Ken Thompson 开发，并使用了 dynamic hashing scheme 来作为 index。BSD 的开发者扩展了 `dbm` library，并将扩展后的 library 称为 `ndbm`。而 `ndbm` functions 已经被标准化了，是 SUS 的 XSI option。

[Seltzer, M., and Yigit, O. 1991. "A New Hashing Packet for UNIX," *Proceedings of the 1991 Winter USENIX Conference*, pp.173-184, Dallas, TX.] 详细地说明了用在 `dbm` library 中的 dynamic hashing algorithm 的历史，以及该 library 的其他 implementations，包括 `gdbm`（`dbm` library 的 GNU 版本）。但是，这些 implementations 的限制是它们不支持多个进程同时更新 the database，它们没有提供对并发的控制（例如 record locking）。

4.4BSD 提供了一个新的 `db`(3) library，它支持 3 中不同的 access forms：(a) record oriented；(b) hashing 和 (c) a B-tree。同样，`db` 也不支持并发（在 `db`(3) manual page 中的 BUGS section 有说明）。

大部分商业的 database libraries 都支持多个进程同时更新一个 database。这些商业的 systems 通常都使用了 advisory locking（Section 14.3），并且大多都使用 B+ trees 或其他的 dynamic hashing technique（例如 linear hashing 或  extendible hashing）来实现他们的 database。

- 想了解 B+ tree，看 [Comer, D. E. 1979. "The Ubiquitous B-Tree," *ACM Computing Surveys,* vol. 11, no. 2, pp. 121–137 (June).]，这是一篇对 B-tree 进行了全面描述的 paper。
- 想了解 linear hashing，看 [Litwin, W. 1980. “Linear Hashing: A New Tool for File and Table Addressing," *Proceedings of the 6th International Conference on Very Large Databases*, pp. 212–223, Montreal, Canada.]，这是一篇描述 linear hashing technique 的 paper。
- 想了解 extendible hashing，看 [Fagin, R., Nievergelt, J., Pippenger, N., and Strong, H. R. 1979. "Extendible Hashing—A Fast Access Method for Dynamic Files," *ACM Transactions on Databases,* vol. 4, no. 3, pp. 315–344 (September).]，这是一篇描述 extendible hashing technique 的 paper。

## Implementation Overview

Database access libraries 通常使用 2 个文件来存储 information：一个 index file 和一个 data file。该 index file 包含了真正的 index value（即 the key）和一个 pointer，该 pointer 指向在 data file 中对应的 data record。在 index file 中，可以用 hashing 和 B+ trees 来提高查找 key 的效率。

Index file 和 data file 的文件布局：

```
+-----------------------------------------------------------------------------------------------------------------------+
|  index file:                                                                                                          |
|                                                                                                                       |
| offset of            +                   hash table                 +                index records              +     |
| first record <---+   | <------------------------------------------> | <---------------------------------------> |     |
| on free list     |   +                                              +                                           +     |
|                  |                                                                                                    |
|           +------+---+-----------+-----------+-----+-----------+----+-------------------------------------------+     |
|           |          |           |           |     |           |    |                                           |     |
|           | free ptr | chain ptr | chain ptr | ... | chain ptr | \n |                                           |     |
|           |          |           |           |     |           |    |                                           |     |
|           +----------+-----------+---------+-+-----+-----------+------------------------------------------------+     |
|                       offset of            |                        |                                           |     |
|                       next index record <--+                        |                                           |     |
|                       on this hash chain                            |                                           |     |
|            +--------------------------------------------------------+                                           |     |
|            |                                                                                                    |     |
+-----------------------------------------------------------------------------------------------------------------------+
             |                                                                                                    |
             |                +                               one index record                              +     |
             |                | <-------------------------------------------------------------------------> |     |
             v                +                                                                             |     v
                                                    + <------------------+ idx len +----------------------> +
                                                    |
             +----------------+-----------+----------------------------+-----+---------+-----+---------+----+-----+
             |                |           |         |                  |     |         |     |         |    |     |
             |      ...       | chain ptr | idx len |        key       | sep | dat off | sep | dat len | \n | ... |
             |                |        +  |         |                  |     |         |     |         |    |     |
             +----------------+-----------+---------+------------------+-----+-+-------+-----+---------+----+-----+
                                       |                                       |
                        offset of  <---+                          +------------+
                        next index record                         |
                        on this hash chain                        |
+------------------------------------------------------------------------------------------------------------------------+
|  data file:                                                     v                                                      |
|                                                                                                                        |
|                                         +-----------------------+------------+----+---------------------+              |
|                                         |                       |            |    |                     |              |
|                                         |         ...           |    data    | \n |         ...         |              |
|                                         |                       |            |    |                     |              |
|                                         +-----------------------+------------+----+---------------------+              |
|                                                                                                                        |
|                                                                 <----------------->                                    |
|                                                                   one data record                                      |
|                                                                   and dat len                                          |
|                                                                                                                        |
+------------------------------------------------------------------------------------------------------------------------+

```

上图使用 [http://asciiflow.com/](http://asciiflow.com/) 制作。这里我们选择了固定长度的 hash table 来组织 index file，并使用 chaining 来解决 hashing collision 问题。

## Concurrency

我们需要能够对 index file 和 data file 这两个文件进行 locking。有很多种对文件进行 locking 的方法。

### Coarse-Grained Locking（粗粒度上锁）

最简单的 locking 是将这两个文件中的一个作为整个 database 的 lock，并要求 caller 需要获得该 lock 之后才能访问 database。这种 locking 称为 *coarse-grained locking*。

粗粒度上锁的问题是，它限制了并发性。如果一个进程正在向一条 hash chain 添加一个 record，则其他进程应该可以在另一条 hash chain 上进行 read record。

### Fine-Grained Locking（细粒度上锁）

细粒度上锁可以有更好的并发性。

有关并发的详情可以查看 [Date, C. J. 2004. *An Introduction to Database Systems, Eighth Edition.* Addison-Wesley, Boston, MA.] 的 16 章，该书全面地描述了 database systems。
