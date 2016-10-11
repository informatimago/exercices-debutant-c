SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

all:list
list:list.c

test:list
	./list

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf list

.PHONY: test clean clean-all
