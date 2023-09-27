CC=gcc
CFLAGS=-Wall -I.

all: lab0.out

lab0.out: lab0.o
	$(CC) $(CFLAGS) -o $@ $<

lab0.o: lab0.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f core *.o a.out