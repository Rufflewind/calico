clang -DB=${B-8} -DLOOKUP_METHOD=${LOOKUP_METHOD-binary_sorted_search} -DNDEBUG -g -DPROFILE -O3 -Wall src/btree.c && tools/profile.py 25
