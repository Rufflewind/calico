import glob, os, re, subprocess
from makegen import *

TEST_FLAGS = ["-g", "-Wall", "-Wextra", "-Wconversion", "-pedantic",
              "-std=c11", "-D_POSIX_C_SOURCE=199309L"]

def run_test(program_ruleset):
    return simple_command("valgrind $(VALGRINDFLAGS) {0}",
        "check_" + os.path.basename(program_ruleset.default_target),
        [program_ruleset], phony=True)

build_dir_cmds = set()
build_cmds = []
build_deps = []
check_deps = []
srcs = glob.glob("src/*.[ch]") + glob.glob("src/*/*.[ch]")
for src in srcs:
    srcname, srcext = os.path.splitext(src)
    finalsrc = src
    relpath = re.match("^src/(.*)", src).group(1)
    if srcname.endswith("_in"):
        finalsrc = srcname[:-3] + srcext
        with open(finalsrc, "wb") as f:
            f.write(subprocess.check_output(["tools/pp", "preproc", src]))
    if srcname.endswith("_test") and src.endswith(".c"):
        out_fn = "tmp/check_" + \
                 os.path.splitext(relpath)[0][:-5].replace("/", "-")
        check_deps.append(
            run_test(build_program(out_fn, [
                compile_source(
                    src,
                    extra_flags=TEST_FLAGS,
                ),
            ])))
    with open(src, "rt") as f:
        public = "/*@#public*/" in f.read()
    if public:
        inclpath = os.path.join("include", relpath)
        build_deps.append(src)
        d = os.path.dirname(inclpath)
        build_dir_cmds.add(d)
        while True:
            d = os.path.dirname(d)
            if d in ("", "/"):
                break
            try:
                build_dir_cmds.remove(d)
            except KeyError:
                pass
        build_cmds.append("cp {src} {inclpath}".format(**locals()))
build_cmds = (
    sorted(("mkdir -p " + p for p in build_dir_cmds)) +
    build_cmds
)

build_rule = simple_command(
    build_cmds,
    "build",
    build_deps,
    phony=True)

check_rule = alias("check", check_deps)

doc_rule = simple_command("doxygen", "doc", [build_rule], phony=True)

r = alias("all", [
    build_rule,
    check_rule,
]).merge(
    doc_rule,
)
r.cleans.update([
    "include",
])
r.save()
