import io, os, re, subprocess, sys

#@snip/WorkDir[
#@requires: mod:os
class WorkDir(object):
    '''A context manager for changing to a different working directory.  The
    original working directory is restored upon exiting the context.'''

    def __init__(self, path):
        '''Initialize a context for changing the working directory.'''
        self.path = path

    def __enter__(self):
        '''Change the working directory and return the path to the previous
        working directory.'''
        self.prevdir = os.getcwd()
        os.chdir(self.path)
        return self.prevdir

    def __exit__(self, type, value, traceback):
        '''Restore the the temporary directory.'''
        os.chdir(self.prevdir)
#@]

#@snip/load_file[
#@requires: mod:io
def load_file(filename, binary=False, encoding=None,
              errors=None, newline=None):
    '''Read the contents of a file.'''
    mode = "r" + ("b" if binary else "")
    with io.open(filename, mode, encoding=encoding,
                 errors=errors, newline=newline) as stream:
        return stream.read()
#@]

def pexec(args):
    subprocess.check_call(["sh", "-c", 'exec "$@"', "-"] + list(args))

def escape_newlines(s):
    return s.replace("\n", " \\\n")

def replace_ident(s, old, new):
    return re.sub(r"\b{0}\b".format(re.escape(old)), lambda _: new, s)

def prepend_prefix(s,
                   prefix_macro="Prefix",
                   public_pattern="",
                   exclude_pattern="",
                   private_subprefix="priv"):
    idents = get_global_idents(s)
    for ident in idents:
        if re.match("({0})$".format(exclude_pattern), ident):
            continue
        subprefix = private_subprefix
        if re.match("({0})$".format(public_pattern), ident):
            subprefix = ""
        s = replace_ident(
            s,
            ident,
            "cal_cat({0}, {1}_{2})".format(prefix_macro, subprefix, ident),
        )
    return s

def get_all_files(dir):
    for dn, _, bns in os.walk(dir):
        for bn in bns:
            fn = os.path.join(dn, bn)
            if os.path.exists(fn):
                yield fn

class FileGenerator(object):

    def __init__(self, filename):
        self.filename = filename
        self.module = {}
        with open(filename) as f:
            exec(compile(f.read(), filename, "exec"), self.module)
        self.deps = self.module["__deps__"]
        self.main = self.module["main"]

class Preprocessor(object):

    def __init__(self, filename, stream=None):
        self.filename = os.path.abspath(filename)
        self.stream = stream
        self.attributes = {
            "deps": [],
        }

    def preprocess(self):
        if self.stream:
            write = self.stream.write
        else:
            write = lambda s: None
        s = load_file(self.filename)
        with WorkDir(os.path.dirname(self.filename)):
            write(self._process_directives(s))

    def bench(self):
        self.attributes["bench"] = True

    def public(self):
        self.attributes["public"] = True

    def depends(self, deps):
        self.attributes["deps"].extend(deps)

    def _process_directives_replacement(self, _m):
        _body, = _m.groups()
        if _body.lstrip().startswith("#"):
            return _m.group(0)
        stream, sys.stdout = sys.stdout, io.StringIO()
        locals = {"self": self}
        try:
            exec(_body, {}, locals)
        finally:
            stream, sys.stdout = sys.stdout, stream
        return stream.getvalue()

    def _process_directives(self, s):
        return re.sub(
            r"(?s)/\*@(.*?)\*/", self._process_directives_replacement, s)

class TemplateMaker(object):

    def __init__(self,
                 impl_filename,
                 prefix_macro="Prefix",
                 public_pattern="",
                 exclude_pattern="",
                 private_subprefix="priv",
                 params=[]):
        self.impl_filename = impl_filename
        self.prefix_macro = prefix_macro
        self.public_pattern = public_pattern
        self.exclude_pattern = exclude_pattern
        self.private_subprefix = private_subprefix
        self.params = params

        self.identifiers_filename = "{0}_ids.txt".format(
            os.path.splitext(impl_filename)[0])
        self.deps = [self.identifiers_filename]

    def main(self):
        s = []
        params = ["Prefix"] + list(self.params)
        s.append("/*@self.public()*/\n")
        for param in params:
            param = param.split("=", 1)
            if len(param) == 1:
                param, = param
                default = None
            else:
                param, default = param
            s.append("#ifndef {0}\n".format(param))
            if default is None:
                s.append("#error Macro must be defined before "
                         "including this header: {0}\n".format(param))
            else:
                s.append("#define {0} {1}\n".format(param, default))
            s.append("#endif\n")
        idents = load_file(self.identifiers_filename).split()
        for ident in idents:
            if re.match("({0})$".format(self.exclude_pattern), ident):
                continue
            subprefix = self.private_subprefix
            if re.match("({0})$".format(self.public_pattern), ident):
                subprefix = ""
            s.append("#define {0} {1}\n".format(
                ident,
                "cal_cat({0}, {1}_{2})"
                .format(self.prefix_macro, subprefix, ident),
            ))
        s.append('#include "{0}"\n'.format(self.impl_filename))
        for ident in idents:
            if re.match("({0})$".format(self.exclude_pattern), ident):
                continue
            s.append("#undef {0}\n".format(ident))
        for param in params:
            s.append("#undef {0}\n".format(param.split("=", 1)[0]))

        s = "".join(s).rstrip()
        sys.stdout.write(s)
        sys.stdout.write("\n")
