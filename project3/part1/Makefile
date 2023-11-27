CC=gcc
CFLAGS=-g -pthread -lpthread
DEPS=account.h string_parser.h

all: bank

bank: bank.o string_parser.o
	$(CC) $(CFLAGS) -o $@ $^

bank.o: bank.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

string_parser.o: string_parser.c string_parser.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f bank *.o