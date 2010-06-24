import os
import sys


#don't need
sys.exit(0)
tr = os.environ.get("THRILLE_ROOT")
assert not (tr is None)
logger = os.path.join(tr, "src", "serializer", "logger.cpp")
assert os.path.exists(logger)

rsched = open(logger, "r").readlines()

newlogger = []


for x in rsched:
    if "    return rand();" in x:
        newlogger.append("    struct timeval tp;\n")
        newlogger.append("    gettimeofday(&tp, NULL);\n")
        newlogger.append("    return tp.tv_usec;\n")
        continue
    newlogger.append(x)

fout = open(logger, "w")
for x in newlogger:
    fout.write(x)
fout.close()


#for x in rsched:
#    if "        chosenThread = rand()" in x:
#        newrandact.append("        struct timeval tp;\n")
#        newrandact.append("        gettimeofday(&tp, NULL);\n")
#        tmp = "        chosenThread = tp.tv_usec % numberOfChoices;\n"
#        newrandact.append(tmp)
#        continue
#
#    if "        int preempt = rand()" in x:
#        newrandact.append("        struct timeval tc;\n")
#        newrandact.append("        gettimeofday(&tc, NULL);\n")
#        tmp = "        int preempt = tc.tv_usec % chanceOfPreempt;\n"
#        newrandact.append(tmp)
#        continue
#   
#    newrandact.append(x)
#
#fout = open(randsched, "w")
#for x in newrandact:
#    fout.write(x)
#
#fout.close()

