SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

all:reverse-lines-by-character
reverse-lines-by-character:reverse-lines-by-character.c

test:reverse-lines-by-character
	LC_ALL=en_US.ISO8859-1 LC_CTYPE=en_US.ISO8859-1 ./reverse-lines-by-character < wang.txt > result-wang.txt
	LC_ALL=en_US.ISO8859-1 LC_CTYPE=en_US.ISO8859-1  diff result-wang.txt expected-wang.txt
	LC_ALL=en_US.UTF-8     LC_CTYPE=en_US.UTF-8      ./reverse-lines-by-character < input.txt > result.txt
	LC_ALL=en_US.UTF-8     LC_CTYPE=en_US.UTF-8      diff result.txt expected-result.txt

clean:
	-rm -rf *.dSYM *.o result.txt

clean-all:clean
	-rm -rf reverse-lines-by-character

.PHONY: test clean clean-all
