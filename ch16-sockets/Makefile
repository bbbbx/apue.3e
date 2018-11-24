ruptimed: apue.h apue.c daemonize.c set_cloexec.c initserver.c ruptimed.c
	cc apue.c daemonize.c set_cloexec.c initserver.c ruptimed.c -o ruptimed

ruptime: apue.c ruptime.c connect_retry.c
	cc apue.c ruptime.c connect_retry.c -o ruptime

all: ruptime ruptimed
	echo OK
