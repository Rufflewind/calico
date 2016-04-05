## Headers

Note: when including, all headers should be prefixed with `calico/`.

### Ordinary headers

  - `bsearch.h`: typeless binary search

### Template headers

  - `cmp_Type.h` defines a comparison function using the built-in semantics
    (hence, it only works on fundamental types).

  - `cmpg_Type.h` (requires `cmp_Type` defined via `cmp_Type.h` or manually)
    defines a typeless comparison function.

  - `bsearch_Type` (requires `cmpg_Type` defined via `cmpg_Type.h` or manually)
    defines a set of typed binary search functions.

### Compatibility headers

  - `compat/alignas_begin.h` and `compat/alignas_end.h`: define `alignas` if
    available.  Otherwise, unless `RF_ALIGNAS_OPTIONAL` is defined, fail with
    an `#error`.

  - `compat/inline_begin.h` and `compat/inline_end.h`: define `inline` if
    available.  Otherwise, fall back to `static`.

  - `compat/noreturn_begin.h` and `compat/noreturn_end.h`: define `noreturn`
    if available.  Otherwise, fall back to nothing.

  - `compat/static_assert_begin.h` and `compat/static_assert_end.h`: define
    `static_assert` if available.  Otherwise, fall back to some trickery.

## Usage

### Template headers

All template headers that accept a type expect the macro `Type` to be in the
form:

~~~c
#define Type(x) my_type x
~~~

One can optionally specify `Type_Suffix` to change the suffix on the
identifiers (this is necessary for types with names consisting of more than
one token).

The macros, if explicitly defined, need to undefined afterwards manually.

Typical usage:

~~~c
#define Type(x) int x
#include <calico/cmp_Type.h>
#include <calico/bsearch_Type.h>
#undef Type
~~~

### Scoped headers

These headers come in `_begin` and `_end` pairs.  Generally it is a good idea
to keep the scope of these headers as small as possible to avoid clashing with
other headers.

Typical usage in a header file:

~~~c
#ifndef MY_INCLUDE_GUARD
#define MY_INCLUDE_GUARD
#include <stddef.h>
/* other headers ... */
#include <calico/compat/foobar_begin.h>
#ifdef __cplusplus
extern "C" {
#endif

/* header code ... */

#ifdef __cplusplus
}
#endif
#include <calico/compat/foobar_end.h>
#endif
~~~

Typical usage in a non-header file:

~~~c
#include <stdio.h>
/* other headers ... */
#include <calico/compat/foobar_begin.h>

/* source code ... */

// optional: #include <calico/compat/foobar_end.h>
~~~
