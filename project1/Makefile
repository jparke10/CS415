CC=gcc
CFLAGS=-g -Wall
OBJECTS=command.o command_parser.o main.o string_parser.o

all: pseudo-shell

pseudo-shell: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c command_parser.h string_parser.h
	$(CC) $(CFLAGS) -c $<

command.o: command.c string_parser.h command.h
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm pseudo-shell $(OBJECTS)
