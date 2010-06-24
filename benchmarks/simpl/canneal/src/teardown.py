import os
import sys

#don't need this script
sys.exit(0)

tr = os.environ.get("THRILLE_ROOT")
assert not (tr is None)
logger = os.path.join(tr, "src", "serializer", "logger.cpp")
assert os.path.exists(logger)

rsched = open(logger, "r").readlines()

newlogger = []

for x in rsched:
    if "    struct timeval tp;" in x:
        continue
    if "    gettimeofday(&tp, NULL);" in x:
        continue
    if "    return tp.tv_usec;" in x:
        newlogger.append("    return rand();\n")      
        continue

    newlogger.append(x)

fout = open(logger, "w")
for x in newlogger:
    fout.write(x)
fout.close()
