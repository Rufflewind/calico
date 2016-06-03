#!/usr/bin/env python
import subprocess, sys
import numpy as np

def parse_kvs(s):
    d = {}
    for line in s.split("\n"):
        if "=" not in line:
            continue
        k, v = line.split("=", 1)
        d[k] = v
    return d

def print_stats(name, ts, N):
    tmean = np.mean(ts) * 1e9 / N
    tstd = np.std(ts) * 1e9 / N
    tsdom = tstd / len(ts) ** .5
    print("{0} = {1:.1f} +/- {2:.1f} ns; std = {3:.1f}"
          .format(name, tmean, tsdom, tstd))

N = int(sys.argv[1])
MAX = int(sys.argv[2])
its = []
lts = []
slts = []
dts = []
SUFFIX = "{0}_{0}".format(MAX)
for i in range(N):
    d = parse_kvs(subprocess.check_output(["./a.out"]).decode("utf-8"))
    its.append(float(d["time_random_inserts_" + SUFFIX]))
    lts.append(float(d["time_random_lookups_" + SUFFIX]))
    n = int(d["len_" + SUFFIX])
    slts.append(float(d["time_sequential_lookups_" + SUFFIX]))
    dts.append(float(d["time_random_deletes_" + SUFFIX]))
    ndts = int(d["count_random_deletes_" + SUFFIX])

print(n)
print_stats("inserts", its, MAX)
print_stats("lookups", lts, MAX)
print_stats("seq_lookups", slts, n)
print_stats("deletes", dts, ndts)
