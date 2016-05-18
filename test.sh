"${CC-clang}" -g -Wall -Wextra -pedantic -std=gnu99 src/btree.c && valgrind ./a.out
