CC=gcc
CFLAGS=-g
DEPS=hellomake.h
OBJ=hellomake.o hellofunc.o
NAME=hello

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(NAME)