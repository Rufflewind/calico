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
        hint = {
            "bench": False,
            "deps": [],
            "public": False,
        }
        ppdeps = []
        with open(fn, "rt") as f:
            s = f.read(4096)
            if "/*@#bench*/" in s:
                hint["bench"] = True
            # these are object file dependencies (needed by linker)
            m = re.search(r"/\*@#public([^*]*)\*/", s)
            if m:
                arg = m.group(1)
                if arg.startswith(":"):
                    hint["public"] = arg[1:]
                else:
                    hint["public"] = True
            # these are object file dependencies (needed by linker)
            m = re.search(r"/\*@#depends:([^*]*)\*/", s)
            if m:
                hint["deps"] = m.group(1).split()
            # these are preprocessor dependencies
            m = re.search(r"/\*@(depends\([^*]*)\*/", s)
            if m:
                depends = lambda x: x
                ppdeps = [os.path.join(os.path.dirname(fn), x)
                          for x in eval(m.group(1))]
        m = re.match("(.*)_in$", name)
        if m:
            out_name, = m.groups()
            out_fn = os.path.join(os.path.dirname(fn), out_name + ext)
            yield simple_command(
                ["tools/preprocess preproc >$@ " + fn],
                out_fn,
                [fn] + ppdeps,
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
        if ext == ".cpp":
            test_name += "++"
        check_rule = make_run_test_rule(build_program("tmp/test-" + test_name, [
            compile_source(
                src_fn,
                extra_flags=("$(TESTCPPFLAGS) " +
                             ("$(TESTCXXFLAGS)"
                              if src_fn.endswith(".cpp")
                              else "$(TESTCFLAGS)")),
                suffix="",
                extension_suffix=True,
            ) for src_fn in src_fns
        ])).merge(src_rule, hint_merger=do_nothing)
        bench_rule = None
        if src_rule.hint["bench"]:
            bench_rule = build_program("tmp/bench-" + test_name, [
                compile_source(
                    src_fn,
                    extra_flags=("$(BENCHCPPFLAGS) " +
                                 ("$(BENCHCXXFLAGS)"
                                  if src_fn.endswith(".cpp")
                                  else "$(BENCHCFLAGS)")),
                    suffix="_bench",
                    extension_suffix=True,
                ) for src_fn in src_fns
            ]).merge(src_rule, hint_merger=do_nothing)
        yield check_rule, bench_rule

def make_build_rules(src_rules, root, destdir, fakesrc=False):
    for src_rule in src_rules:
        if not src_rule.hint["public"]:
            continue
        if not fakesrc or src_rule.hint["public"] == True:
            fn = src_rule.default_target
        else:
            # when fakesrc is True, we allow the source file name to be
            # overridden by the argument of the "public" directive;
            # we use this when generating documentation
            fn = snormpath(os.path.join(
                os.path.dirname(src_rule.default_target),
                src_rule.hint["public"]))
        out_fn = os.path.join(
            destdir,
            os.path.relpath(src_rule.default_target, root))
        out_dir = os.path.dirname(out_fn)
        yield simple_command(
            "cp {fn} {out_fn}".format(**locals()),
            out_fn,
            [fn],
        ).merge(src_rule, hint_merger=do_nothing)

root = "src"

srcs = get_all_files(root)

ppedsrc_rules = list(make_preprocess_rules(srcs, [".c", ".cpp", ".h"]))

test_rules = list(make_test_rules(ppedsrc_rules, [".c", ".cpp"], root))

build_rule = alias("build", list(
    make_build_rules(ppedsrc_rules, root, "include")))

build_docs_rule = list(
    make_build_rules(ppedsrc_rules, root, "tmp/doc-src/calico", fakesrc=True))

build_bench_rule = alias("build-bench", [x[1] for x in test_rules
                                         if x[1] is not None])

check_rule = alias("check", [x[0] for x in test_rules])

doc_rule = simple_command([
    "tools/generate-doxygen-mainpage <README.md >tmp/doc-src/README.md",
    "doxygen",
], "doc", build_docs_rule, phony=True)

alias("all", [
    build_rule,
    build_bench_rule,
]).merge(
    check_rule,
    doc_rule,
    Ruleset(macros={
        "BENCHCPPFLAGS": "-g -Wall -O3 -DBENCH -DNDEBUG",
        "TESTCPPFLAGS": ("-g -Wall -Wextra -Wconversion -pedantic "
                         "-D_POSIX_C_SOURCE=199309L"),
        "TESTCFLAGS": "-std=c11",
        "TESTCXXFLAGS": "-std=c++11",
    }),
).save()
