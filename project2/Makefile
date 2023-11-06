CC=gcc
CFLAGS=-g
DEPS=MCP.h
NAMES=part1 part2 part3 part4

all: $(NAMES)

$(NAMES): %: %.c $(DEPS)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(NAMES)