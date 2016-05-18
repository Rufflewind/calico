#!/usr/bin/env python
import subprocess, sys
import numpy as np

def parse_kvs(s):
    d = {}
    for line in s.split("\n"):
        if not line:
            continue
        k, v = line.split("=", 1)
        d[k] = v
    return d

N = int(sys.argv[1])
ts = []
for i in range(N):
    d = parse_kvs(subprocess.check_output(["./a.out"]).decode("utf-8"))
    ts.append(float(d["time_random_inserts_1000000_1000000"]))

tmean = np.mean(ts) * 1e3
tstd = np.std(ts) * 1e3
tsdom = tstd / len(ts) ** .5
print("{0:.1f} +/- {1:.1f} ns; std = {2:.1f}".format(tmean, tsdom, tstd))
