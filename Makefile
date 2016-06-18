BENCHCPPFLAGS=-g -Wall -O3 -DBENCH -DNDEBUG
TESTCFLAGS=-std=c11
TESTCPPFLAGS=-g -Wall -Wextra -Wconversion -pedantic -D_POSIX_C_SOURCE=199309L
TESTCXXFLAGS=-std=c++11

all: build build-bench check

build: include/arithmetic.h include/binary_search.h include/btree.h include/btree_head.h include/btree_template.h include/compat/alignas_begin.h include/compat/alignas_end.h include/compat/alloca.h include/compat/inline_begin.h include/compat/inline_end.h include/compat/noreturn_begin.h include/compat/noreturn_end.h include/compat/restrict_begin.h include/compat/restrict_end.h include/compat/static_assert_begin.h include/compat/static_assert_end.h include/linear_ordered_search.h include/macros.h include/shuffle.h include/wclock.h

build-bench: tmp/bench-btree tmp/bench-stdmap++

check: run-test-binary_search run-test-btree run-test-btree++ run-test-shuffle run-test-stdmap++ run-test-wclock

clean:
	rm -fr -- include/arithmetic.h include/binary_search.h include/btree.h include/btree_head.h include/btree_template.h include/compat/alignas_begin.h include/compat/alignas_end.h include/compat/alloca.h include/compat/inline_begin.h include/compat/inline_end.h include/compat/noreturn_begin.h include/compat/noreturn_end.h include/compat/restrict_begin.h include/compat/restrict_end.h include/compat/static_assert_begin.h include/compat/static_assert_end.h include/linear_ordered_search.h include/macros.h include/shuffle.h include/wclock.h src/arithmetic.h src/binary_search_test.c.o src/black_box.c.o src/black_box_bench.c.o src/btree.h src/btree_template.h src/btree_test.c.o src/btree_test.cpp.o src/btree_test_bench.c.o src/shuffle_test.c.o src/stdmap_test.cpp.o src/stdmap_test_bench.cpp.o src/wclock_test.c.o tmp/bench-btree tmp/bench-stdmap++ tmp/test-binary_search tmp/test-btree tmp/test-btree++ tmp/test-shuffle tmp/test-stdmap++ tmp/test-wclock

doc: build
	doxygen

run-test-binary_search: tmp/test-binary_search
	valgrind $(VALGRINDFLAGS) tmp/test-binary_search

run-test-btree: tmp/test-btree
	valgrind $(VALGRINDFLAGS) tmp/test-btree

run-test-btree++: tmp/test-btree++
	valgrind $(VALGRINDFLAGS) tmp/test-btree++

run-test-shuffle: tmp/test-shuffle
	valgrind $(VALGRINDFLAGS) tmp/test-shuffle

run-test-stdmap++: tmp/test-stdmap++
	valgrind $(VALGRINDFLAGS) tmp/test-stdmap++

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

src/binary_search_test.c.o: src/binary_search.h src/binary_search_test.c src/compat/inline_begin.h src/compat/inline_end.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCPPFLAGS) $(TESTCFLAGS) -c -o $@ src/binary_search_test.c

src/black_box.c.o: src/black_box.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCPPFLAGS) $(TESTCFLAGS) -c -o $@ src/black_box.c

src/black_box_bench.c.o: src/black_box.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BENCHCPPFLAGS) $(BENCHCFLAGS) -c -o $@ src/black_box.c

src/btree.h: src/btree_in.h
	tools/pp preproc >$@ src/btree_in.h

src/btree_template.h: src/btree_template_in.h
	tools/pp preproc >$@ src/btree_template_in.h

src/btree_test.c.o: src/binary_search.h src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.c src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/utils.h src/wclock.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCPPFLAGS) $(TESTCFLAGS) -c -o $@ src/btree_test.c

src/btree_test.cpp.o: src/btree.hpp src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.cpp src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TESTCPPFLAGS) $(TESTCXXFLAGS) -c -o $@ src/btree_test.cpp

src/btree_test_bench.c.o: src/binary_search.h src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.c src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/utils.h src/wclock.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BENCHCPPFLAGS) $(BENCHCFLAGS) -c -o $@ src/btree_test.c

src/shuffle_test.c.o: src/shuffle.h src/shuffle_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCPPFLAGS) $(TESTCFLAGS) -c -o $@ src/shuffle_test.c

src/stdmap_test.cpp.o: src/compat/inline_begin.h src/compat/inline_end.h src/stdmap_test.cpp src/utils.h src/wclock.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TESTCPPFLAGS) $(TESTCXXFLAGS) -c -o $@ src/stdmap_test.cpp

src/stdmap_test_bench.cpp.o: src/compat/inline_begin.h src/compat/inline_end.h src/stdmap_test.cpp src/utils.h src/wclock.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(BENCHCPPFLAGS) $(BENCHCXXFLAGS) -c -o $@ src/stdmap_test.cpp

src/wclock_test.c.o: src/compat/inline_begin.h src/compat/inline_end.h src/wclock.h src/wclock_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(TESTCPPFLAGS) $(TESTCFLAGS) -c -o $@ src/wclock_test.c

tmp/bench-btree: src/black_box_bench.c.o src/btree_test_bench.c.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/btree_test_bench.c.o src/black_box_bench.c.o

tmp/bench-stdmap++: src/black_box_bench.c.o src/stdmap_test_bench.cpp.o
	@mkdir -p tmp
	$(CXX) $(CXXFLAGS) -o $@ src/stdmap_test_bench.cpp.o src/black_box_bench.c.o

tmp/test-binary_search: src/binary_search_test.c.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/binary_search_test.c.o

tmp/test-btree: src/black_box.c.o src/btree_test.c.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/btree_test.c.o src/black_box.c.o

tmp/test-btree++: src/btree_test.cpp.o
	@mkdir -p tmp
	$(CXX) $(CXXFLAGS) -o $@ src/btree_test.cpp.o

tmp/test-shuffle: src/shuffle_test.c.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/shuffle_test.c.o

tmp/test-stdmap++: src/black_box.c.o src/stdmap_test.cpp.o
	@mkdir -p tmp
	$(CXX) $(CXXFLAGS) -o $@ src/stdmap_test.cpp.o src/black_box.c.o

tmp/test-wclock: src/wclock_test.c.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/wclock_test.c.o

.PHONY: all build build-bench check clean doc run-test-binary_search run-test-btree run-test-btree++ run-test-shuffle run-test-stdmap++ run-test-wclock
