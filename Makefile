BENCHFLAGS=-g -Wall -O3 -DBENCH -DNDEBUG
TESTFLAGS=-g -Wall -Wextra -Wconversion -pedantic -std=c11 -D_POSIX_C_SOURCE=199309L

all: build build-bench check

build: include/arithmetic.h include/binary_search.h include/btree.h include/btree_head.h include/btree_template.h include/compat/alignas_begin.h include/compat/alignas_end.h include/compat/alloca.h include/compat/inline_begin.h include/compat/inline_end.h include/compat/noreturn_begin.h include/compat/noreturn_end.h include/compat/restrict_begin.h include/compat/restrict_end.h include/compat/static_assert_begin.h include/compat/static_assert_end.h include/linear_ordered_search.h include/macros.h include/shuffle.h include/wclock.h

build-bench: tmp/bench-btree

check: run-test-binary_search run-test-btree run-test-shuffle run-test-wclock

clean:
	rm -fr -- include/arithmetic.h include/binary_search.h include/btree.h include/btree_head.h include/btree_template.h include/compat/alignas_begin.h include/compat/alignas_end.h include/compat/alloca.h include/compat/inline_begin.h include/compat/inline_end.h include/compat/noreturn_begin.h include/compat/noreturn_end.h include/compat/restrict_begin.h include/compat/restrict_end.h include/compat/static_assert_begin.h include/compat/static_assert_end.h include/linear_ordered_search.h include/macros.h include/shuffle.h include/wclock.h src/arithmetic.h src/binary_search_test.o src/black_box.o src/black_box_bench.o src/btree.h src/btree_template.h src/btree_test.o src/btree_test_bench.o src/shuffle_test.o src/wclock_test.o tmp/bench-btree tmp/test-binary_search tmp/test-btree tmp/test-shuffle tmp/test-wclock

doc: build
	doxygen

run-test-binary_search: tmp/test-binary_search
	valgrind $(VALGRINDFLAGS) tmp/test-binary_search

run-test-btree: tmp/test-btree
	valgrind $(VALGRINDFLAGS) tmp/test-btree

run-test-shuffle: tmp/test-shuffle
	valgrind $(VALGRINDFLAGS) tmp/test-shuffle

run-test-wclock: tmp/test-wclock
	valgrind $(VALGRINDFLAGS) tmp/test-wclock

include/arithmetic.h: src/arithmetic.h
	@mkdir -p include
	cp src/arithmetic.h include

include/binary_search.h: src/binary_search.h
	@mkdir -p include
	cp src/binary_search.h include

include/btree.h: src/btree.h
	@mkdir -p include
	cp src/btree.h include

include/btree_head.h: src/btree_head.h
	@mkdir -p include
	cp src/btree_head.h include

include/btree_template.h: src/btree_template.h
	@mkdir -p include
	cp src/btree_template.h include

include/compat/alignas_begin.h: src/compat/alignas_begin.h
	@mkdir -p include/compat
	cp src/compat/alignas_begin.h include/compat

include/compat/alignas_end.h: src/compat/alignas_end.h
	@mkdir -p include/compat
	cp src/compat/alignas_end.h include/compat

include/compat/alloca.h: src/compat/alloca.h
	@mkdir -p include/compat
	cp src/compat/alloca.h include/compat

include/compat/inline_begin.h: src/compat/inline_begin.h
	@mkdir -p include/compat
	cp src/compat/inline_begin.h include/compat

include/compat/inline_end.h: src/compat/inline_end.h
	@mkdir -p include/compat
	cp src/compat/inline_end.h include/compat

include/compat/noreturn_begin.h: src/compat/noreturn_begin.h
	@mkdir -p include/compat
	cp src/compat/noreturn_begin.h include/compat

include/compat/noreturn_end.h: src/compat/noreturn_end.h
	@mkdir -p include/compat
	cp src/compat/noreturn_end.h include/compat

include/compat/restrict_begin.h: src/compat/restrict_begin.h
	@mkdir -p include/compat
	cp src/compat/restrict_begin.h include/compat

include/compat/restrict_end.h: src/compat/restrict_end.h
	@mkdir -p include/compat
	cp src/compat/restrict_end.h include/compat

include/compat/static_assert_begin.h: src/compat/static_assert_begin.h
	@mkdir -p include/compat
	cp src/compat/static_assert_begin.h include/compat

include/compat/static_assert_end.h: src/compat/static_assert_end.h
	@mkdir -p include/compat
	cp src/compat/static_assert_end.h include/compat

include/linear_ordered_search.h: src/linear_ordered_search.h
	@mkdir -p include
	cp src/linear_ordered_search.h include

include/macros.h: src/macros.h
	@mkdir -p include
	cp src/macros.h include

include/shuffle.h: src/shuffle.h
	@mkdir -p include
	cp src/shuffle.h include

include/wclock.h: src/wclock.h
	@mkdir -p include
	cp src/wclock.h include

src/arithmetic.h: src/arithmetic_in.h
	tools/pp preproc >$@ src/arithmetic_in.h

src/binary_search_test.o: src src/binary_search.h src/binary_search_test.c src/compat/inline_begin.h src/compat/inline_end.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTFLAGS) -c -o $@ src/binary_search_test.c

src/black_box.o: src/black_box.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTFLAGS) -c -o $@ src/black_box.c

src/black_box_bench.o: src/black_box.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BENCHFLAGS) -c -o $@ src/black_box.c

src/btree.h: src/btree_in.h
	tools/pp preproc >$@ src/btree_in.h

src/btree_template.h: src/btree_template_in.h
	tools/pp preproc >$@ src/btree_template_in.h

src/btree_test.o: src src/binary_search.h src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.c src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/utils.h src/wclock.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTFLAGS) -c -o $@ src/btree_test.c

src/btree_test_bench.o: src src/binary_search.h src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.c src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/utils.h src/wclock.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BENCHFLAGS) -c -o $@ src/btree_test.c

src/shuffle_test.o: src/shuffle.h src/shuffle_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTFLAGS) -c -o $@ src/shuffle_test.c

src/wclock_test.o: src src/compat/inline_begin.h src/compat/inline_end.h src/wclock.h src/wclock_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTFLAGS) -c -o $@ src/wclock_test.c

tmp/bench-btree: src/black_box_bench.o src/btree_test_bench.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/btree_test_bench.o src/black_box_bench.o

tmp/test-binary_search: src/binary_search_test.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/binary_search_test.o

tmp/test-btree: src/black_box.o src/btree_test.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/btree_test.o src/black_box.o

tmp/test-shuffle: src/shuffle_test.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/shuffle_test.o

tmp/test-wclock: src/wclock_test.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/wclock_test.o

.PHONY: all build build-bench check clean doc run-test-binary_search run-test-btree run-test-shuffle run-test-wclock
