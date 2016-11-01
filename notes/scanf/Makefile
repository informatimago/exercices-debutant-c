SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

NAME=scanf

all:$(NAME)

$(NAME):$(NAME).c
test:$(NAME)
	( echo 1 ; echo 2 ; echo 3 ; echo hello ; echo 4 ; echo 0) | ./$(NAME)

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf $(NAME)

.PHONY: test clean clean-all
