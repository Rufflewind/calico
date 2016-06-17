all: build check

build: src/arithmetic_in.h src/binary_search.h src/btree_head.h src/btree_in.h src/btree_template_in.h src/compat/alignas_begin.h src/compat/alignas_end.h src/compat/alloca.h src/compat/inline_begin.h src/compat/inline_end.h src/compat/noreturn_begin.h src/compat/noreturn_end.h src/compat/restrict_begin.h src/compat/restrict_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/wclock.h
	mkdir -p include/compat
	cp src/macros.h include/macros.h
	cp src/wclock.h include/wclock.h
	cp src/btree_head.h include/btree_head.h
	cp src/btree_in.h include/btree_in.h
	cp src/linear_ordered_search.h include/linear_ordered_search.h
	cp src/shuffle.h include/shuffle.h
	cp src/binary_search.h include/binary_search.h
	cp src/btree_template_in.h include/btree_template_in.h
	cp src/arithmetic_in.h include/arithmetic_in.h
	cp src/compat/static_assert_end.h include/compat/static_assert_end.h
	cp src/compat/inline_end.h include/compat/inline_end.h
	cp src/compat/noreturn_begin.h include/compat/noreturn_begin.h
	cp src/compat/restrict_end.h include/compat/restrict_end.h
	cp src/compat/noreturn_end.h include/compat/noreturn_end.h
	cp src/compat/inline_begin.h include/compat/inline_begin.h
	cp src/compat/static_assert_begin.h include/compat/static_assert_begin.h
	cp src/compat/alloca.h include/compat/alloca.h
	cp src/compat/alignas_begin.h include/compat/alignas_begin.h
	cp src/compat/restrict_begin.h include/compat/restrict_begin.h
	cp src/compat/alignas_end.h include/compat/alignas_end.h

check: check_check_binary_search check_check_btree check_check_shuffle check_check_wclock

check_check_binary_search: tmp/check_binary_search
	valgrind $(VALGRINDFLAGS) tmp/check_binary_search

check_check_btree: tmp/check_btree
	valgrind $(VALGRINDFLAGS) tmp/check_btree

check_check_shuffle: tmp/check_shuffle
	valgrind $(VALGRINDFLAGS) tmp/check_shuffle

check_check_wclock: tmp/check_wclock
	valgrind $(VALGRINDFLAGS) tmp/check_wclock

clean:
	rm -fr -- include src/binary_search_test_fDEDyVXDaiYbNGd27mPd.o src/btree_test_fDEDyVXDaiYbNGd27mPd.o src/shuffle_test_fDEDyVXDaiYbNGd27mPd.o src/wclock_test_fDEDyVXDaiYbNGd27mPd.o tmp/check_binary_search tmp/check_btree tmp/check_shuffle tmp/check_wclock

doc: build
	doxygen

src/binary_search_test_fDEDyVXDaiYbNGd27mPd.o:  src/binary_search.h src/binary_search_test.c src/compat/inline_begin.h src/compat/inline_end.h
	$(CC) $(CPPFLAGS) $(CFLAGS)  -g -Wall -Wextra -Wconversion -pedantic -std=c11 -D_POSIX_C_SOURCE=199309L -c -o $@ src/binary_search_test.c

src/btree_test_fDEDyVXDaiYbNGd27mPd.o:  src/binary_search.h src/btree_head.h src/btree_impl.h src/btree_template.h src/btree_test.c src/compat/inline_begin.h src/compat/inline_end.h src/compat/static_assert_begin.h src/compat/static_assert_end.h src/linear_ordered_search.h src/macros.h src/shuffle.h src/utils.h src/wclock.h
	$(CC) $(CPPFLAGS) $(CFLAGS)  -g -Wall -Wextra -Wconversion -pedantic -std=c11 -D_POSIX_C_SOURCE=199309L -c -o $@ src/btree_test.c

src/shuffle_test_fDEDyVXDaiYbNGd27mPd.o: src/shuffle.h src/shuffle_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS)  -g -Wall -Wextra -Wconversion -pedantic -std=c11 -D_POSIX_C_SOURCE=199309L -c -o $@ src/shuffle_test.c

src/wclock_test_fDEDyVXDaiYbNGd27mPd.o:  src/compat/inline_begin.h src/compat/inline_end.h src/wclock.h src/wclock_test.c
	$(CC) $(CPPFLAGS) $(CFLAGS)  -g -Wall -Wextra -Wconversion -pedantic -std=c11 -D_POSIX_C_SOURCE=199309L -c -o $@ src/wclock_test.c

tmp/check_binary_search: src/binary_search_test_fDEDyVXDaiYbNGd27mPd.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/binary_search_test_fDEDyVXDaiYbNGd27mPd.o

tmp/check_btree: src/btree_test_fDEDyVXDaiYbNGd27mPd.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/btree_test_fDEDyVXDaiYbNGd27mPd.o

tmp/check_shuffle: src/shuffle_test_fDEDyVXDaiYbNGd27mPd.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/shuffle_test_fDEDyVXDaiYbNGd27mPd.o

tmp/check_wclock: src/wclock_test_fDEDyVXDaiYbNGd27mPd.o
	@mkdir -p tmp
	$(CC) $(CFLAGS) -o $@ src/wclock_test_fDEDyVXDaiYbNGd27mPd.o

.PHONY: all build check check_check_binary_search check_check_btree check_check_shuffle check_check_wclock clean doc
