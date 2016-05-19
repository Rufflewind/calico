"${CC-clang}" -DNDEBUG -g -DPROFILE -O3 -Wall "$@" src/btree_test.c src/dummy.c && tools/bench.py "${count-25}"
