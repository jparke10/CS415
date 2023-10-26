CC=gcc
CFLAGS=-g -Wall

all: lab5

lab5: lab5.o iobound
	$(CC) $(CFLAGS) -o $@ $<

lab5.o: lab5.c
	$(CC) $(CFLAGS) -c $<

iobound: iobound.c
	$(CC) -o $@ $<

clean:
	rm *.o iobound lab5
