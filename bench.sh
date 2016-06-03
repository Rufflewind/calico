"${CC-clang}" -DNDEBUG -DMAX="${MAX-10000}" -g -DBENCH -O3 -Wall "$@" src/btree_test.c src/dummy.c && tools/bench.py "${count-25}" "${MAX-10000}"
