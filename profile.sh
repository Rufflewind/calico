"${CC-clang}" -DNDEBUG -pg -g -DPROFILE -O3 -Wall -fno-omit-frame-pointer "$@" src/btree_test.c src/dummy.c && ./a.out && gprof
