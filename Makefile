all:help
help::
	@echo make zip
	@echo make compile-solutions
	@echo make test-solutions

TMP     := $(shell echo $${TMPDIR-$${TMP-/tmp}})
ZIPNAME := exercices-$$(date +%Y%m%d)

zip: ../$(ZIPNAME).zip

../$(ZIPNAME).zip:
	mkdir $(TMP)/$(ZIPNAME)
	tar pocf - * | tar -C $(TMP)/$(ZIPNAME)  -xvf -
	cd $(TMP) ; zip -r $(ZIPNAME).zip $(ZIPNAME)
	mv -v $(TMP)/$(ZIPNAME).zip ..
	rm -rf $(TMP)/$(ZIPNAME)

PROGRAMS = \
	solutions/batons \
	solutions/character-of-season \
	solutions/list \
	solutions/reverse-file \
	solutions/reverse-lines-by-byte \
	solutions/reverse-lines-by-character \
	notes/arithmetic \
	notes/scanf-vs-atoi \
	notes/funbool \
	travaux-pratiques/plus-ou-moins 


compile:
	@for program in $(PROGRAMS) ; do \
		echo ==== $$program ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C $$program all ; done

test:
	@for program in $(PROGRAMS) ; do \
		echo ==== $$program ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C $$program test ; done

clean clean-all:
	@for program in $(PROGRAMS) ; do \
		echo ==== $$program ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C $$program $@ ; done

.PHONY: zip compile test
