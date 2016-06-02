test: src/btree.h src/btree_template.h
	./test.sh

clean:
	rm -f src/arithmetic.h src/btree.h src/btree_template.h

.PHONY: clean test

src/arithmetic.h: src/arithmetic.h.py
	src/arithmetic.h.py >$@.tmp
	mv $@.tmp $@

src/btree.h: src/btree_in.h
	tools/pp preproc src/btree_in.h >$@.tmp
	mv $@.tmp $@

src/btree_template.h: src/btree_template_in.h
	tools/pp preproc src/btree_template_in.h >$@.tmp
	mv $@.tmp $@
