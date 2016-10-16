SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

NAME=funbool

all:$(NAME)

$(NAME):$(NAME).c
test:$(NAME)
	./$(NAME)

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf $(NAME)

.PHONY: test clean clean-all
