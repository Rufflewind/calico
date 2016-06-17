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
        with open(fn, "rt") as f:
            public = "/*@#public*/" in f.read(4096)
        m = re.match("(.*)_in$", name)
        if m:
            out_name, = m.groups()
            out_fn = os.path.join(os.path.dirname(fn), out_name + ext)
            yield simple_command(
                ["tools/pp preproc >$@ " + fn],
                out_fn,
                [fn],
                hint={"public": public},
            )
        else:
            yield plain_file(fn, hint={"public": public})

TEST_FLAGS = ["-g", "-Wall", "-Wextra", "-Wconversion", "-pedantic",
              "-std=c11", "-D_POSIX_C_SOURCE=199309L"]

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
        out_fn = ("tmp/test-" +
                  snormpath(os.path.join(
                      os.path.relpath(os.path.dirname(fn), root),
                      out_name
                  )).replace("/", "-"))
        print(build_program(out_fn, [
            compile_source(
                fn,
                extra_flags=TEST_FLAGS,
                no_mangle=True,
            ),
        ]))
        yield make_run_test_rule(build_program(out_fn, [
            compile_source(
                fn,
                extra_flags=TEST_FLAGS,
                no_mangle=True,
            ),
        ])).merge(src_rule, hint_merger=do_nothing)

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

check_rule = alias("check", list(test_rules))

doc_rule = simple_command("doxygen", "doc", [build_rule], phony=True)

alias("all", [
    build_rule,
    check_rule,
]).merge(
    doc_rule,
).save()
