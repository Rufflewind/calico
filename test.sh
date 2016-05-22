"${CC-clang}" -g -Wall -Wextra -Wconversion -pedantic -std=gnu11 "$@" src/btree_test.c && valgrind ${VALGRINDFLAGS-} ./a.out
