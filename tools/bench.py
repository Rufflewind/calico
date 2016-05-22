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

def print_stats(name, ts):
    tmean = np.mean(ts) * 1e9 / 1e4
    tstd = np.std(ts) * 1e9 / 1e4
    tsdom = tstd / len(ts) ** .5
    print("{0} = {1:.1f} +/- {2:.1f} ns; std = {3:.1f}"
          .format(name, tmean, tsdom, tstd))

N = int(sys.argv[1])
its = []
lts = []
for i in range(N):
    d = parse_kvs(subprocess.check_output(["./a.out"]).decode("utf-8"))
    its.append(float(d["time_random_inserts_10000_10000"]))
    lts.append(float(d["time_random_lookups_10000_10000"]))

print_stats("inserts", its)
print_stats("lookups", lts)
