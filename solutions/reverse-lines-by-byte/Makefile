# -*- coding:utf-8 -*-
SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

all:reverse-lines-by-byte
reverse-lines-by-byte:reverse-lines-by-byte.c

test:reverse-lines-by-byte
	@echo "// Puisque wang.txt est encodé en ISO8859-1 (un caractère = un octet),"
	@echo "// la transformation suivante sera correcte:"
	LC_ALL=en_US.ISO8859-1 \
	LC_CTYPE=en_US.ISO8859-1 \
	./reverse-lines-by-byte < wang.txt > result-wang.txt
	LC_ALL=en_US.ISO8859-1 \
	LC_CTYPE=en_US.ISO8859-1 \
	diff result-wang.txt expected-wang.txt
	@echo "// Cependant, le fichier input.txt est encodé en UTF-8 (un caractère"
	@echo "// peut être encodé sur plus d\'un octet), dont cette transformation"
	@echo "// produira un résultat incorrect:"
	LC_ALL=en_US.UTF-8 \
	LC_CTYPE=en_US.UTF-8 \
	./reverse-lines-by-byte < input.txt > result.txt
	LC_ALL=en_US.UTF-8 \
	LC_CTYPE=en_US.UTF-8 \
	diff result.txt expected-result.txt

clean:
	-rm -rf *.dSYM *.o result-wang.txt

clean-all:clean
	-rm -rf reverse-lines-by-byte

.PHONY: test clean clean-all
