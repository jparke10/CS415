CC=gcc
CFLAGS=-g -Wall
OBJECTS=command.o command_parser.o main.o string_parser.o

all: pseudo-shell

pseudo-shell: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm pseudo-shell $(OBJECTS)
