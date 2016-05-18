clang -DB=8 -DNDEBUG -g -DPROFILE -O3 -Wall src/btree.c && tools/profile.py 25
