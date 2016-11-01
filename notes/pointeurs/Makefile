SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

NAME=pointeurs

all:$(NAME)

$(NAME):$(NAME).c
test:$(NAME)
	printf "%d\n" 1 2 3 5 7 11 13 | ./$(NAME)

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf $(NAME)

.PHONY: test clean clean-all
