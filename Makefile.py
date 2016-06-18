import os, re, subprocess
from makegen import *

def get_all_files(dir):
    for dn, _, bns in os.walk(dir):
        for bn in bns:
            yield os.path.join(dn, bn)

def make_preprocess_rules(fns, extensions):
    for fn in fns:
        name, ext = os.path.splitext(os.path.basename(fn))
        if ext not in extensions:
            continue
        hint = {"deps": [], "public": False}
        with open(fn, "rt") as f:
            s = f.read(4096)
            if "/*@#public*/" in s:
                hint["public"] = True
            m = re.search(r"/\*@#depends:([^*]*)\*/", s)
            if m:
                hint["deps"] = m.group(1).split()
        m = re.match("(.*)_in$", name)
        if m:
            out_name, = m.groups()
            out_fn = os.path.join(os.path.dirname(fn), out_name + ext)
            yield simple_command(
                ["tools/pp preproc >$@ " + fn],
                out_fn,
                [fn],
                hint=hint,
            )
        else:
            yield plain_file(fn, hint=hint)

def make_run_test_rule(program_ruleset):
    return simple_command("valgrind $(VALGRINDFLAGS) {0}",
        "run-" + os.path.basename(program_ruleset.default_target),
        [program_ruleset], phony=True)

def make_test_rules(src_rules, extensions, root):
    for src_rule in src_rules:
        fn = src_rule.default_target
        name, ext = os.path.splitext(os.path.basename(fn))
        if ext not in extensions:
            continue
        m = re.match("(.*)_test$", name)
        if not m:
            continue
        out_name, = m.groups()
        dn = os.path.dirname(fn)
        src_fns = [fn] + [snormpath(os.path.join(dn, bn))
                          for bn in src_rule.hint["deps"]]
        test_name = snormpath(os.path.join(
            os.path.relpath(dn, root),
            out_name
        )).replace("/", "-")
        yield (
            make_run_test_rule(build_program("tmp/test-" + test_name, [
                compile_source(
                    src_fn,
                    extra_flags="$(TESTFLAGS)",
                    suffix="",
                ) for src_fn in src_fns
            ])).merge(src_rule, hint_merger=do_nothing),
            build_program("tmp/bench-" + test_name, [
                compile_source(
                    src_fn,
                    extra_flags="$(BENCHFLAGS)",
                    suffix="_bench",
                ) for src_fn in src_fns
            ]).merge(src_rule, hint_merger=do_nothing),
        )

def make_build_rules(src_rules, root):
    for src_rule in src_rules:
        if not src_rule.hint["public"]:
            continue
        fn = src_rule.default_target
        out_fn = os.path.join("include", os.path.relpath(fn, root))
        out_dir = os.path.dirname(out_fn)
        yield simple_command(
            "cp {fn} {out_dir}".format(**locals()),
            out_fn,
            [fn],
        ).merge(src_rule, hint_merger=do_nothing)

root = "src"

srcs = get_all_files(root)

ppedsrc_rules = list(make_preprocess_rules(srcs, [".c", ".h"]))

test_rules = list(make_test_rules(ppedsrc_rules, [".c"], root))

build_rule = alias("build", list(make_build_rules(ppedsrc_rules, root)))

build_bench_rule = alias("build-bench", [x[1] for x in test_rules])

check_rule = alias("check", [x[0] for x in test_rules])

doc_rule = simple_command("doxygen", "doc", [build_rule], phony=True)

alias("all", [
    build_rule,
    build_bench_rule,
    check_rule,
]).merge(
    doc_rule,
    Ruleset(macros={
        "BENCHFLAGS": "-g -Wall -O3 -DBENCH -DNDEBUG",
        "TESTFLAGS": ("-g -Wall -Wextra -Wconversion -pedantic "
                       "-std=c11 -D_POSIX_C_SOURCE=199309L"),
    }),
).save()
