SHELL=/bin/bash
CFLAGS=-g3 -ggdb

PROGRAMMES = \
	plus-ou-moins \
	plus-ou-moins-cnt \
	plus-ou-moins-cnt-rpt \
	plus-ou-moins-2

all:$(PROGRAMMES)


test:$(PROGRAMMES)
	./plus-ou-moins
	./plus-ou-moins-cnt
	./plus-ou-moins-cnt-rpt
	./plus-ou-moins-2

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf $(PROGRAMMES)

.PHONY: test clean clean-all
