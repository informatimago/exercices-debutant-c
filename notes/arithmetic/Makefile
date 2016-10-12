SHELL=/bin/bash
CFLAGS=-g3 -ggdb

NAME=arithmetic

all:$(NAME)

$(NAME):$(NAME).c

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf $(NAME)

.PHONY: test clean clean-all
