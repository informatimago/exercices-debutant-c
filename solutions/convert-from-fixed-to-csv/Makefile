SHELL=/bin/bash
CFLAGS=-g3 -ggdb

all:convert-from-fixed-to-csv
convert-from-fixed-to-csv:convert-from-fixed-to-csv.c

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf convert-from-fixed-to-csv

.PHONY: test clean clean-all
