all: test

build: src/btree.h src/btree_template.h

clean:
	rm -f src/arithmetic.h src/btree.h src/btree_template.h

doc: build
	rm -r tmp/doc-src
	mkdir tmp/doc-src
	cp src/btree_impl.h tmp/doc-src/btree_template.h
	doxygen

test: build
	$(CC) -g -Wall -Wextra -Wconversion -pedantic -std=gnu11 $(ARGS) src/btree_test.c && valgrind $(VALGRINDFLAGS) ./a.out

.PHONY: all build clean doc test

src/arithmetic.h: src/arithmetic.h.py
	src/arithmetic.h.py >$@.tmp
	mv $@.tmp $@

src/btree.h: src/btree_in.h src/btree_impl.h
	tools/pp preproc src/btree_in.h >$@.tmp
	mv $@.tmp $@

src/btree_template.h: src/btree_template_in.h src/btree_impl.h
	tools/pp preproc src/btree_template_in.h >$@.tmp
	mv $@.tmp $@
