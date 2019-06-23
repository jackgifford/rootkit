KMOD=	icmp
SRCS= 	icmp.c

all: spawn.o
	cc -o spawn spawn.o

spawn.o: 
	cc -c spawn.c 

rem:	clean
	rm spawn.o spawn 

.include <bsd.kmod.mk>
