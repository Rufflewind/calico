#!/usr/bin/env python
import subprocess
import numpy as np

def parse_kvs(s):
    d = {}
    for line in s.split("\n"):
        if not line:
            continue
        k, v = line.split("=", 1)
        d[k] = v
    return d

N = 10
ts = []
for i in range(N):
    d = parse_kvs(subprocess.check_output(["./a.out"]).decode("utf-8"))
    ts.append(float(d["time_random_inserts_1000000_1000000"]))

print(np.mean(ts) * 1e3, "+/-", np.std(ts) * 1e3, "ns")
