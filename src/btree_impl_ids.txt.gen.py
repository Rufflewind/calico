import utils

__deps__ = ["btree_impl.h"]

def main():
    utils.pexec(["parse-global-identifiers"] + __deps__)
