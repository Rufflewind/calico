#include <stddef.h>
#include <functional>
#include <stdexcept>
#include <utility>
#include "btree_head.h"
/*@#public*/

namespace cal {

/* used to enable the empty-base optimization without causing conflicts
   with members of the actual class */
template<class Base, class T>
struct ebo_wrapper : Base {
    T value;
    ebo_wrapper(const Base &base, const T &value) : Base(base), value(value) {}
};

template<class Key,
         class T = void,
         std::size_t MinArity = 8,
         class Compare = std::less<Key> >
class btree : Compare {
public:

    typedef Key key_type;
    typedef T mapped_type;
    typedef mapped_type value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef Compare key_compare;

private:

#define Prefix
#define KeyType key_type
#define ValueType value_type
#define MinArity MinArity
#define CompareFunction _cmp
#define SearchFunction linear_ordered_search
#include "btree_template.h"

    typedef ebo_wrapper<key_compare, _btree> _internal_type;
    _internal_type _m;

    static int _cmp(const void *ctx, const key_type *x, const key_type *y)
    {
        const _internal_type &_m =
            *reinterpret_cast<const _internal_type *>(
                static_cast<const char *>(ctx)
                - offsetof(_internal_type, value));
        const key_compare &lt = static_cast<const key_compare &>(_m);
        return lt(*x, *y) ? -1 : lt(*y, *x);
    }

    bool _insert(Key &&key, T &&value, T *oldvalue_out)
    {
        alignas(alignof(Key)) char kbuf[sizeof(Key)];
        alignas(alignof(T)) char vbuf[sizeof(T)];
        Key *const k = new(kbuf) Key(std::move(key));
        T *const v = new(vbuf) T(std::move(value));
        const int r = _btree_insert(&_m.value, k, v, v);
        if (r < 0) {
            k->~Key();
            v->~T();
            throw std::runtime_error("out of memory");
        } else if (r) {
            k->~Key();
            if (oldvalue_out) {
                *oldvalue_out = std::move(*v);
            } else {
                v->~T();
            }
            return true;
        }
        return false;
    }

public:

    explicit btree(const Compare &comp = Compare()) : _m(comp, _btree()) {
        _btree_init(&_m.value);
    }

    ~btree() {
        _btree_entry entry;
        for (_btree_find_first(&_m.value, &entry);
             _btree_entry_occupied(&_m.value, &entry);
             _btree_entry_next(&_m.value, &entry)) {
printf("%i\n", (int)entry._istack[entry._depth]);
printf("%i\n", (int)*priv_btree_leaf_len(entry._nodestack[entry._depth]));

            _btree_entry_key(&entry)->~Key();
            _btree_entry_get(&entry)->~T();
        }
        _btree_reset(&_m.value);
    }

    size_type size() const {
        return _btree_len(&_m.value);
    }

    const T *get(const Key &key) const
    {
        return _btree_get_const(&_m.value, &key);
    }

    T *get(const Key &key)
    {
        return _btree_get(&_m.value, &key);
    }

    const T &at(const Key &key) const
    {
        return *get(key);
    }

    T &at(const Key &key)
    {
        return *get(key);
    }

    bool insert(Key key, T value)
    {
        return _insert(std::move(key), std::move(value), NULL);
    }

    bool insert(Key key, T value, T &old_value_out)
    {
        return _insert(std::move(key), std::move(value), &old_value_out);
    }

    bool erase(const Key &key)
    {
        alignas(alignof(Key)) char kbuf[sizeof(Key)];
        alignas(alignof(T)) char vbuf[sizeof(T)];
        Key *const k = reinterpret_cast<Key *>(kbuf);
        T *const v = reinterpret_cast<T *>(vbuf);
        const bool r = static_cast<bool>(_btree_remove(&_m.value, &key, k, v));
        if (r) {
            k->~Key();
            v->~T();
        }
        return r;
    }

};

}
