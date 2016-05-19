"${CC-clang}" -g -Wall -Wextra -pedantic -std=gnu99 src/btree_test.c && valgrind ./a.out
