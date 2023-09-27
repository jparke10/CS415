CC=gcc
CFLAGS=-Wall -I.

all: a.out

a.out: hello.o
	$(CC) $(CFLAGS) $<

hello.o: hello.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f core *.o a.out