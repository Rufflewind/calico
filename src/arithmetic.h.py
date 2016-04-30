#!/usr/bin/env python
import sys

header_begin = """
#ifndef G_O01SKTDA3IE3J0XOJ0SFUII39F91W
#define G_O01SKTDA3IE3J0XOJ0SFUII39F91W
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#ifndef __has_feature
# define __has_feature(x) 0
#endif
#ifndef __has_builtin
# define __has_builtin(x) 0
#endif
#ifdef __cplusplus
extern "C" {
#endif
"""[1:]

header_end = """
#ifdef __cplusplus
}
#endif
#endif
"""[1:]

qualifiers = "static inline"

wrap_operations = set(["add", "sub", "mul"])

binop_wrapper_simple = lambda body, builtin: """
#if __GNUC__ >= 5 || __has_builtin(__builtin_{builtin}_overflow)
    return __builtin_{builtin}_overflow(x, y, z);
#else
{body}#endif
"""[1:].format(body=body, builtin=builtin)

binop_wrapper_typedef = lambda body, builtin, type, impl_name, entries: """
#if __GNUC__ >= 5 || __has_builtin(__builtin_{builtin}_overflow)
    return __builtin_{builtin}_overflow(x, y, z);
#elif __has_feature(c_generic_selections)
    return _Generic(
        ({type})0,
{entries}        default: {impl_name}
    )(z, x, y);
#else
    return {impl_name}(z, x, y);
#endif
"""[1:].format(body=body, builtin=builtin, type=type,
               impl_name=impl_name, entries=entries)

binop_guard_typedef_entry = lambda operation, type: """
        {type}: {name},
"""[1:].format(type=type, name=construct_name(operation, type))

docs = lambda operation, type: {

    "unsafe_divfloor": """
/** Calculate floored division and modulus where the quotient is rounded
    towards minus infinity.  UNSAFE: the result must not overflow. */
"""[1:],

    "divfloor": """
/** Calculate floored division and modulus where the quotient is rounded
    towards minus infinity. */
"""[1:],

}.get(operation, "")

operation_body = lambda operation, type: {

    "s:add": """
    if ((y > 0 && x > {const}_MAX - y) || (y < 0 && x < {const}_MIN - y)) {{
        return 1;
    }}
    *z = x + y;
    return 0;
"""[1:].format(const=const_prefix(type)),

    "s:neg": """
    if (-{const}_MAX != {const}_MIN && x == {const}_MIN) {{
        return 1;
    }}
    *z = -x;
    return 0;
"""[1:].format(const=const_prefix(type)),

    "s:sub": """
    if ((y > 0 && x < {const}_MIN + y) || (y < 0 && x > {const}_MAX + y)) {{
        return 1;
    }}
    *z = x - y;
    return 0;
"""[1:].format(const=const_prefix(type)),

    "s:mul": """
    if (x > 0) {{
        if (y > 0) {{
            if (x > {const}_MAX / y) {{
                return 1;
            }}
        }} else {{
            if (y < {const}_MIN / x) {{
                return 1;
            }}
        }}
    }} else {{
        if (y > 0) {{
            if (x < {const}_MIN / y) {{
                return 1;
            }}
        }} else {{
            if (x && y < {const}_MAX / x) {{
                return 1;
            }}
        }}
    }}
    *z = x * y;
    return 0;
"""[1:].format(const=const_prefix(type)),

    "s:div": """
    if (!y || (-{const}_MAX != {const}_MIN && x == {const}_MIN && y == -1)) {{
        return 1;
    }}
    *z = {func}(x, y).quot;
    return 0;
"""[1:].format(const=const_prefix(type), func=div_func(type)),

    # in C11, modulo is undefined if its corresponding division is not
    # representable (6.5.5)
    "s:mod": """
    if (!y || (-{const}_MAX != {const}_MIN && x == {const}_MIN && y == -1)) {{
        return 1;
    }}
    *z = {func}(x, y).rem;
    return 0;
"""[1:].format(const=const_prefix(type), func=div_func(type)),

    # The behavior of integer division and modulus was implementation-defined
    # until C99/C++11.  To strictly conform to earlier standards, we instead
    # use the `div` function, which has well-defined behavior.  Note: if `div`
    # succeeds, then neither of the two operations in the `if` block can
    # overflow.
    "s:unsafe_divfloor": """
    {div_type} r = {div_func}(x, y);
    if (r.rem && (x < 0) != (y < 0)) {{
        --r.quot;
        r.rem += y;
    }}
    return r;
"""[1:].format(div_func=div_func(type),
               div_type=div_type(type)),

    "s:divfloor": """
    if (!x || (-{const}_MAX != {const}_MIN && x == {const}_MIN && y == -1)) {{
        return 1;
    }}
    *z = {unsafe_func}(x, y);
    return 0;
"""[1:].format(const=const_prefix(type),
               div_func=div_func(type),
               div_type=div_type(type),
               unsafe_func=construct_name("unsafe_divfloor", type)),

    "s:abs": """
    if (-{const}_MAX != {const}_MIN && x == {const}_MIN) {{
        return 1;
    }}
    *z = {func}(x);
    return 0;
"""[1:].format(const=const_prefix(type), func=abs_func(type)),

    "u:add": """
    if (x > ({type})(-1) - y) {{
        return 1;
    }}
    *z = x + y;
    return 0;
"""[1:].format(type=type),

    "u:sub": """
    if (x < y) {{
        return 1;
    }}
    *z = x - y;
    return 0;
"""[1:].format(),

    "u:mul": """
    if (y && x > ({type})(-1) / y) {{
        return 1;
    }}
    *z = x * y;
    return 0;
"""[1:].format(type=type),

    "u:div": """
    if (!y) {{
        return 1;
    }}
    *z = x / y;
    return 0;
"""[1:].format(),

    "u:mod": """
    if (!y) {{
        return 1;
    }}
    *z = x % y;
    return 0;
"""[1:].format(const=const_prefix(type)),

}.get(("u" if type in unsigned_types else "s") + ":" + operation, None)

all_operations = [
    "add",
    "sub",
    "mul",
    "div",
    "unsafe_divfloor",
    "divfloor",
    "mod",
    "abs",
]

prototype = lambda operation, type, name: {
    "add": "int {name}({type} *z, {type} x, {type} y)",
    "sub": "int {name}({type} *z, {type} x, {type} y)",
    "mul": "int {name}({type} *z, {type} x, {type} y)",
    "div": "int {name}({type} *z, {type} x, {type} y)",
    "mod": "int {name}({type} *z, {type} x, {type} y)",
    "abs": "int {name}({type} *z, {type} x)",
    "divfloor": "int {name}({div_type} *z, {type} x, {type} y)",
    "unsafe_divfloor": "{div_type} {name}({type} x, {type} y)",
}[operation].format(type=type, name=name, div_type=div_type(type))

all_types = [
    "int",
    "long",
    "long long",
    "unsigned",
    "unsigned long",
    "unsigned long long",
    "size_t",
]

const_prefix = lambda type: {
    "int": "INT",
    "long": "LONG",
    "long long": "LLONG",
    "unsigned": "UINT",
    "unsigned long": "ULONG",
    "unsigned long long": "ULLONG",
    "size_t": "SIZE",
}[type]

abs_func = lambda type: {
    "int": "abs",
    "long": "labs",
    "long long": "llabs",
}.get(type, None)

div_func = lambda type: {
    "int": "div",
    "long": "ldiv",
    "long long": "lldiv",
}.get(type, None)

div_type = lambda type: {
    "int": "div_t",
    "long": "ldiv_t",
    "long long": "lldiv_t",
}.get(type, None)

unsigned_types = set([
    "unsigned",
    "unsigned long",
    "unsigned long long",
    "size_t",
])

guard = lambda type: {
    "long long": [const_prefix("long long") + "_MAX"],
    "unsigned long long": [const_prefix("unsigned long long") + "_MAX"],
}.get(type, [])

builtin_template = {
    "int": "s{0}",
    "long": "s{0}l",
    "long long": "s{0}ll",
    "unsigned": "u{0}",
    "unsigned long": "u{0}l",
    "unsigned long long": "u{0}ll",
    "size_t": "{0}",
}

suffix = {
    "int": "i",
    "long": "l",
    "long long": "ll",
    "unsigned": "u",
    "unsigned long": "ul",
    "unsigned long long": "ull",
    "size_t": "z",
}

aliases = lambda type: {
    "size_t": ["unsigned", "unsigned long", "unsigned long long"],
}.get(type, None)

convtypes = [
    ("long", "int"),
    ("long long", "int"),
    ("long long", "long"),
]

def construct_name(operation, type):
    return operation + "_" + suffix[type]

def construct_conv_name(srctype, dsttype):
    return suffix[srctype] + "_to_" + suffix[dsttype]

write = sys.stdout.write
write(header_begin)
write("\n")

for srctype, dsttype in convtypes:
    cur_guard = guard(srctype) + guard(dsttype)
    name = construct_conv_name(srctype, dsttype)
    if cur_guard:
        write("#ifdef {0}\n".format(" && ".join(cur_guard)))
    if qualifiers:
        write(qualifiers)
        write("\n")
    write("int {name}({dsttype} *z, {srctype} x)"
          .format(name=name, srctype=srctype, dsttype=dsttype))
    write("\n{\n")
    write("""
    if (x < {const}_MIN || x > {const}_MAX) {{
        return 1;
    }}
    *z = ({dsttype})x;
    return 0;
"""[1:].format(dsttype=dsttype, const=const_prefix(dsttype)))
    write("}\n")
    if cur_guard:
        write("#endif\n")
    write("\n")

for operation in all_operations:
    for type in all_types:
        name = construct_name(operation, type)
        body = operation_body(operation, type)
        if not body:
            continue
        builtin = builtin_template[type].format(operation)
        impl_name = name + "_"
        cur_aliases = aliases(type)
        cur_guard = guard(type)
        wrap = operation in wrap_operations
        doc = docs(operation, type)

        if cur_guard:
            write("#ifdef {0}\n".format(" && ".join(cur_guard)))

        if wrap and cur_aliases is not None:
            if qualifiers:
                write(qualifiers)
                write("\n")
            write(prototype(operation, type, impl_name))
            write("\n{\n")
            write(body)
            write("}\n\n")

        if doc:
            write(doc)
        if qualifiers:
            write(qualifiers)
            write("\n")
        write(prototype(operation, type, name))
        write("\n{\n")
        if not wrap:
            write(body)
        elif cur_aliases is None:
            write(binop_wrapper_simple(body, builtin))
        else:
            entries = "".join(binop_guard_typedef_entry(operation, alias_type)
                              for alias_type in cur_aliases)
            write(binop_wrapper_typedef(body, builtin, type,
                                        impl_name, entries))
        write("}\n")

        if cur_guard:
            write("#endif\n")

        write("\n")

write(header_end)
