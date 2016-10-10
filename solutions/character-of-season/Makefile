SHELL=/bin/bash

all:characterOfSeason
characterOfSeason:characterOfSeason.c

test:characterOfSeason
	./characterOfSeason
	./characterOfSeason $$(date +%m) $$(date +%d)
	./characterOfSeason  1 10
	./characterOfSeason  4 10
	./characterOfSeason  7 10
	./characterOfSeason 10 10

test-bash:characterOfSeason.bash
	source characterOfSeason.bash ; characterForSeason $$(date +%m) $$(date +%d)

clean:
	-rm -rf *.dSYM *.o

clean-all:clean
	-rm -rf characterOfSeason

.PHONY: test clean clean-all
