# Communicating with a Network Printer

Applications 一般使用 Internet Printing Protocol（IPP）和 printers 进行通信。

一个将 jobs 发送给 printer 的 print spooler daemon 和一个提交 print jobs 给该 spooler daemon 的 command。

> SPOOL（**S**imultaneous **P**eripheral **O**perations **O**n-line）是一种数据缓冲，指传输数据的过程中，将数据存放在临时工作区中，其它程序可以在之后的任意时间点对其存取。  —— Wikipedia

## The Internet Printing Protocol

IPP 建立在 HTTP 之上。下表是 IPP 的一些 RFC 文档。

|Document| Title                                    |
|:-------|:----------------------------------------|
|RFC 2567| IPP 的 Design Goals  |
|RFC 2568| Rationale for the Structure of the Model and Protocol for the Internet Printing Protocol |
|RFC 2911| Internet Printing Protocol/1.1: Model and Semantics |
|RFC 2910| Internet Printing Protocol/1.1: Encoding and Transport |
|RFC 3196| Internet Printing Protocol/1.1: Implementor’s Guide |
|Candidate Standard 5100.12-2011| Internet Printing Protocol Version 2.0, Second Edition |

IPP 是一种 request–response protocol。一个 client 发送一个 request message 给一个 server，然后该 server 用一个 response message 来 answer 该 client。一个 IPP message 的结构如下。

```txt
+-----------------+-----------+------------+-------------+------------+--------------------+
| Ethernet header | IP header | TCP header | HTTP header | IPP header | data to be printed |
+-----------------+-----------+------------+-------------+------------+--------------------+
```

其中，IPP header 的结构如下。

```txt
+-------------------------+---------------------------------------------------------+---------------------+-----------------------+-------------------------------+
| version number(2 bytes) | operation ID (request)/status code (response) (2 bytes) | request ID(4 bytes) | attributes(0~n bytes) | end-of-attributes tag(1 byte) |
+-------------------------+---------------------------------------------------------+---------------------+-----------------------+-------------------------------+
```

## Printer Spooling

所有的 UNIX Systems 至少都支持一个 print spooling system。FreeBSD 安装的是 LPD（the BSD print spooling system，see `lpd`(8) and Chapter 13 of Stevens [1990]），Linux 和 Mac OS X 预装了 CUPS（Common UNIX Printing System，see `cupsd`(8)）。

Printer spooling 的结构如下。

```c
  +------------------------------------------+
  |                                          v
+-------------+     +---------------+      +-----------------------+      +--------+     +--------+     +--------+
| config file | --> | print command | <--> | printd, print spooler | <--> | file 1 | --> | file 2 | --> | file 3 |
+-------------+     +---------------+      +-----------------------+      +--------+     +--------+     +--------+
                      ^                      ^
                      |                      |
                      |                      |
                      |                      v
                    +---------------+      +-----------------------+
                    | file to print |      |        printer        |
                    +---------------+      +-----------------------+
```