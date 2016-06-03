all: test

build: src/btree.h src/btree_template.h
	mkdir -p include/calico
	cp -r \
	   src/compat \
	   src/btree_head.h \
	   src/btree_template.h \
	   src/macros.h \
	   include/calico

clean:
	rm -f src/arithmetic.h src/btree.h src/btree_template.h

doc: build
	rm -fr tmp/doc-src
	mkdir -p tmp/doc-src/calico
	cp src/btree_impl.h tmp/doc-src/calico/btree_template.h
	cp Doxyfile tmp/doc-src/Doxyfile
	cd tmp/doc-src && doxygen

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
