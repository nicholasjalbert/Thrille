import sys
import os
from math import sqrt


if len(sys.argv) != 2:
    print "python generateSummaryStat.py [simpl-stat file]"

def getAvg(list):
    total = 0
    for x in list:
        total += x
    return (float(total)/float(len(list)))

def getStdDev(list):
    avg = getAvg(list)
    deviation_list = []
    for x in list:
        deviation_list.append(x - avg)
    assert len(deviation_list) == len(list)
    deviation_squared_list = []
    for x in deviation_list:
        deviation_squared_list.append(x*x)
    assert len(deviation_list) == len(deviation_squared_list)
    avgdev = getAvg(deviation_squared_list)
    return sqrt(avgdev)


def printSumStats(str, blocks, cs, pre, non):
    assert getStdDev([3,7,7,19]) == 6
    sblocks = sorted(blocks)
    scs = sorted(cs)
    spre = sorted(pre)
    snon = sorted(non)
    print "For", str, ":"
    print "\tblock:"
    print "\t\tmin:", sblocks[0]
    firstQ = len(sblocks)/4
    firstQ = int(round(firstQ))
    print "\t\t1st Q:", sblocks[firstQ]
    print "\t\tavg:", getAvg(sblocks)
    secondQ = int(round(firstQ*2))
    print "\t\tmedian:", sblocks[secondQ]
    thirdQ = firstQ * 3
    thirdQ = int(round(thirdQ))
    print "\t\t3rd Q:", sblocks[thirdQ]
    print "\t\tmax:", sblocks[-1]
    print "\t\tstd_dev:", getStdDev(sblocks)

    print "\tContext Switch:"
    print "\t\tmin:", scs[0]
    firstQ = len(scs)/4
    firstQ = int(round(firstQ))
    print "\t\t1st Q:", scs[firstQ]
    print "\t\tavg:", getAvg(scs)
    secondQ = int(round(firstQ*2))
    print "\t\tmedian:", scs[secondQ]
    thirdQ = firstQ * 3
    thirdQ = int(round(thirdQ))
    print "\t\t3rd Q:", scs[thirdQ]
    print "\t\tmax:", scs[-1]
    print "\t\tstd_dev:", getStdDev(scs)

    print "\tPreemptions:"
    print "\t\tmin:", spre[0]
    firstQ = len(spre)/4
    firstQ = int(round(firstQ))
    print "\t\t1st Q:", spre[firstQ]
    print "\t\tavg:", getAvg(spre)
    secondQ = int(round(firstQ*2))
    print "\t\tmedian:", spre[secondQ]
    thirdQ = firstQ * 3
    thirdQ = int(round(thirdQ))
    print "\t\t3rd Q:", spre[thirdQ]
    print "\t\tmax:", spre[-1]
    print "\t\tstd_dev:", getStdDev(spre)

    print "\tNonpreemptive switches:"
    print "\t\tmin:", snon[0]
    firstQ = len(snon)/4
    firstQ = int(round(firstQ))
    print "\t\t1st Q:", snon[firstQ]
    print "\t\tavg:", getAvg(snon)
    secondQ = int(round(firstQ*2))
    print "\t\tmedian:", snon[secondQ]
    thirdQ = firstQ * 3
    thirdQ = int(round(thirdQ))
    print "\t\t3rd Q:", snon[thirdQ]
    print "\t\tmax:", snon[-1]
    print "\t\tstd_dev:", getStdDev(snon)
    

fout = open("py-summarystat.csv", "w")
fin = open(sys.argv[1], "r").readlines()

run_count = 0
startblocks = []
startcs = []
startpre = []
startnon = []

naablocks = []
naacs = []
naapre = []
naanon = []

nbbblocks = []
nbbcs = []
nbbpre = []
nbbnon = []

iaablocks = []
iaacs = []
iaapre = []
iaanon = []

ibbblocks = []
ibbcs = []
ibbpre = []
ibbnon = []

while len(fin) > 0:
    item = fin.pop(0)
    if "**RUN" in item:
        assert "Start" in fin.pop(0)
        
        item = fin.pop(0)
        assert "blocks" in item
        item = item.split(":")
        assert len(item) == 2
        startblocks.append(int(item[1].strip()))
        
        item = fin.pop(0)
        assert "context switches" in item
        item = item.split(":")
        assert len(item) == 2
        startcs.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "preemptions" in item
        item = item.split(":")
        assert len(item) == 2
        startpre.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "non-preemptive" in item
        item = item.split(":")
        assert len(item) == 2
        startnon.append(int(item[1].strip()))
       
        assert "NAA" in fin.pop(0)

        item = fin.pop(0)
        assert "blocks" in item
        item = item.split(":")
        assert len(item) == 2
        naablocks.append(int(item[1].strip()))
        
        item = fin.pop(0)
        assert "context switches" in item
        item = item.split(":")
        assert len(item) == 2
        naacs.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "preemptions" in item
        item = item.split(":")
        assert len(item) == 2
        naapre.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "non-preemptive" in item
        item = item.split(":")
        assert len(item) == 2
        naanon.append(int(item[1].strip()))
       
        assert "NBB" in fin.pop(0)

        item = fin.pop(0)
        assert "blocks" in item
        item = item.split(":")
        assert len(item) == 2
        nbbblocks.append(int(item[1].strip()))
        
        item = fin.pop(0)
        assert "context switches" in item
        item = item.split(":")
        assert len(item) == 2
        nbbcs.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "preemptions" in item
        item = item.split(":")
        assert len(item) == 2
        nbbpre.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "non-preemptive" in item
        item = item.split(":")
        assert len(item) == 2
        nbbnon.append(int(item[1].strip()))


        assert "IAA" in fin.pop(0)

        item = fin.pop(0)
        assert "blocks" in item
        item = item.split(":")
        assert len(item) == 2
        iaablocks.append(int(item[1].strip()))
        
        item = fin.pop(0)
        assert "context switches" in item
        item = item.split(":")
        assert len(item) == 2
        iaacs.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "preemptions" in item
        item = item.split(":")
        assert len(item) == 2
        iaapre.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "non-preemptive" in item
        item = item.split(":")
        assert len(item) == 2
        iaanon.append(int(item[1].strip()))
       
        assert "IBB" in fin.pop(0)

        item = fin.pop(0)
        assert "blocks" in item
        item = item.split(":")
        assert len(item) == 2
        ibbblocks.append(int(item[1].strip()))
        
        item = fin.pop(0)
        assert "context switches" in item
        item = item.split(":")
        assert len(item) == 2
        ibbcs.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "preemptions" in item
        item = item.split(":")
        assert len(item) == 2
        ibbpre.append(int(item[1].strip()))

        item = fin.pop(0)
        assert "non-preemptive" in item
        item = item.split(":")
        assert len(item) == 2
        ibbnon.append(int(item[1].strip()))

        run_count += 1

print "Number of runs:",run_count


printSumStats("start", startblocks, startcs, startpre, startnon)
printSumStats("NAA", naablocks, naacs, naapre, naanon)
printSumStats("NBB", nbbblocks, nbbcs, nbbpre, nbbnon)
printSumStats("IAA", iaablocks, iaacs, iaapre, iaanon)
printSumStats("IBB", ibbblocks, ibbcs, ibbpre, ibbnon)
