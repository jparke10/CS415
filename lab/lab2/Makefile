CC=gcc
CFLAGS=-g -Wall

all : a.out

a.out: command.o main.o
	$(CC) $(CFLAGS) $^ 

command.o: command.c
	$(CC) $(CFLAGS) -c $^

main.o: main.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm a.out command.o main.o