SHELL=/bin/bash
# CFLAGS=-g3 -ggdb -DTEST=1

all:batons
batons:batons.c
test:batons
	./batons 1 2 4
	./batons --verbose 1 2 4
	./batons --verbose 40 20 10 1 2 4 7 5 11 13


clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf batons

.PHONY: test clean clean-all
