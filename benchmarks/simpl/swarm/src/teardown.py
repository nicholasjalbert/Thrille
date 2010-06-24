import os
import sys


tr = os.environ.get("THRILLE_ROOT")
assert not (tr is None)
randact_cpp = os.path.join(tr, "src", "randact", "randomactive.cpp")
assert os.path.exists(randact_cpp)

randact = open(randact_cpp, "r").readlines()

newrandact = []

for x in randact:
    if "    chanceOfPreempt = " in x:
        newrandact.append("    chanceOfPreempt = 1;\n")
        continue
    if "    sparsifyNum = " in x:
        newrandact.append("    sparsifyNum = 1;\n")
        continue
    if "    sparsifyDenom = " in x:
        newrandact.append("    sparsifyDenom = 1;\n")
        continue
    
    newrandact.append(x)

fout = open(randact_cpp, "w")
for x in newrandact:
    fout.write(x)

fout.close()

