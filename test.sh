"${CC-clang}" -g -Wall -Wextra -pedantic -std=gnu11 "$@" src/btree_test.c && valgrind ./a.out
