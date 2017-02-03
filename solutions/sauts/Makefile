SHELL=/bin/bash
# CFLAGS=-g3 -ggdb

all:sauts
sauts:sauts.c

test:sauts
	@ v=(2 0 3 6 0 0 3 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts $${v[@]}
	@ v=(2 0 3 5 0 0 3 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts $${v[@]}
	@ v=(2 0 3 5 0 0 2 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts $${v[@]}
	@ v=(1) ; printf "%s " $${v[@]} ; ./sauts $${v[@]}
	@ v=(0) ; printf "%s " $${v[@]} ; ./sauts $${v[@]}
	@ v=(2 0 3 6 0 0 3 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts --path $${v[@]}
	@ v=(2 0 3 5 0 0 3 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts --path $${v[@]}
	@ v=(2 0 3 5 0 0 2 0 0 3 1 0) ; printf "%s " $${v[@]} ; ./sauts --path $${v[@]}
	@ v=(1) ; printf "%s " $${v[@]} ; ./sauts --path $${v[@]}
	@ v=(0) ; printf "%s " $${v[@]} ; ./sauts --path $${v[@]}

clean:
	-rm -rf *.dSYM *.o .DS_Store

clean-all:clean
	-rm -rf sauts

.PHONY: test clean clean-all
