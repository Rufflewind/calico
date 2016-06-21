# Calico

[![Travis CI build status][ti]][tl]
[![AppVeyor build status][ai]][al]

[ti]: https://travis-ci.org/Rufflewind/calico.svg?branch=master
[tl]: https://travis-ci.org/Rufflewind/calico
[ai]: https://ci.appveyor.com/api/projects/status/github/Rufflewind/calico?branch=master&svg=true
[al]: https://ci.appveyor.com/project/Rufflewind/calico

**Quick links**: [documentation](https://rufflewind.com/calico).

## Headers

**Note**: when including, all headers should be prefixed with `calico/`.

### Ordinary headers

  - `arithmetic.h`: checked arithmetic.
  - `binary_search.h`: binary search algorithm.
  - `linear_ordered_search.h`: linear search through an ordered array.
  - `macros.h`: utility macros.
  - `shuffle.h`: random shuffle algorithm.
  - `wclock.h`: monotonic wall clock.

### Template headers

  - `btree_template.h` defines a B-tree data type and its associated
    functions.  It can be used as an associative array container (map).  It
    can also be used as a set, if the macro `Value` is left undefined.

### Compatibility headers

  - `compat/alignas_begin.h` and `compat/alignas_end.h`: define `alignas` if
    available.  Otherwise, unless `RF_ALIGNAS_OPTIONAL` is defined, fail with
    a preprocessor error.

  - `compat/restrict_begin.h` and `compat/restrict_end.h`: define `restrict`
    if available.  Otherwise, fall back to nothing.

  - `compat/inline_begin.h` and `compat/inline_end.h`: define `inline` if
    available.  Otherwise, fall back to `static`.

  - `compat/noreturn_begin.h` and `compat/noreturn_end.h`: define `noreturn`
    if available.  Otherwise, fall back to nothing.

  - `compat/restrict_begin.h` and `compat/restrict_end.h`: define `restrict`
    if available.  Otherwise, fall back to nothing.

  - `compat/static_assert_begin.h` and `compat/static_assert_end.h`: define
    `static_assert` if available.  Otherwise, fall back to some trickery.

## Usage

### Template headers

Template headers are special C headers that can be included multiple times.
They generally expect some arguments, which are supplied through parameter
macros.  As an example, consider the `btree_template.h` header, which can be
used like this:

~~~c
#include <calico/btree_head.h>

#define Prefix foo
#define KeyType int
#define ValueType double
#include <calico/btree_template.h>
~~~

The *associated header* `btree_head.h` *must be included at least once* before
any inclusions of the `btree_template.h` header.  Including `btree_head.h`
more than once is unnecessary but not harmful either.

After including `btree_template.h`, the parameter macros (`Prefix`, `KeyType`,
and `ValueType`) are automatically undefined.

Typically, `Prefix` macro specifies the prefix that is attached to all
identifiers related to the template header.  For example, the example above
causes the type `foo_btree` to be defined, as well as functions such as
`foo_btree_insert`.

### Scoped headers

These headers come in `*_begin.h` and `*_end.h` pairs.  The purpose of such
headers is to minimize the risk of naming collisions, and hence it is a good
idea to keep the scope of these headers as small as possible to avoid clashing
with other headers.  Nested inclusion of the same scoped headers is not
allowed.

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

In a non-header file (`.c` or `.cpp`), one can be more lenient and simply use
the `*_begin.h` without the corresponding `*_end.h`.  For example:

~~~c
#include <stdio.h>
/* other headers ... */
#include <calico/compat/foobar_begin.h>

/* source code ... */
~~~
