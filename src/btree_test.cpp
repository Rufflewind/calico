#include <assert.h>
#include <vector>
#include "btree.hpp"

int main(void)
{
    {
        cal::btree<long, long> t;
        assert(t.size() == 0);
        assert(t.get(2) == NULL);
        t.insert(2, 3);
        assert(t.size() == 1);
        assert(t.at(2) == 3);
        assert(t.get(0) == NULL);
        assert(t.erase(0) == false);
        assert(t.erase(2) == true);
        assert(t.erase(2) == false);
        assert(t.size() == 0);
    }

    {
        std::vector<long> out;
        std::vector<long> a(2);
        a[0] = 12;
        a[1] = 37;
        std::vector<long> b(1);
        a[0] = 42;

        cal::btree<std::vector<long>, std::vector<long> > t;
        assert(t.get(a) == NULL);

        t.insert(a, a);
        assert(t.at(a) == a);

        t.insert(a, b);
        assert(t.at(a) == b);

        t.insert(a, a, out);
        assert(t.at(a) == a);
        assert(out == b);

        assert(t.get(b) == NULL);
        assert(t.erase(b) == false);
        assert(t.erase(a) == true);
    }
    return 0;
}
