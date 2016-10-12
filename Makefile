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

SOLUTIONS=batons character-of-season list reverse-file reverse-lines-by-byte reverse-lines-by-character

compile-solutions:
	@for solution in $(SOLUTIONS) ; do \
		echo ==== $$solution ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C solutions/$$solution all ; done

test-solutions:
	@for solution in $(SOLUTIONS) ; do \
		echo ==== $$solution ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C solutions/$$solution test ; done

clean clean-all:
	@for solution in $(SOLUTIONS) ; do \
		echo ==== $$solution ======================================================================== \
		| sed -e 's/^\(........................................................................\).*/\1/' ;\
		make -C solutions/$$solution $@ ; done

.PHONY: zip compile-solutions test-solutions
