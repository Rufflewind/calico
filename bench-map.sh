"${CXX-clang++}" -DNDEBUG -g -DPROFILE -O3 -Wall "$@" src/baseline.cpp src/dummy.c && tools/bench.py "${count-25}"
