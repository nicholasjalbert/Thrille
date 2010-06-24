import sys
import os
from math import sqrt

def getAvg(list):
    total = 0
    for x in list:
        total += x
    return round(float(total)/float(len(list)), 3)

def getPctRed(start, end):
    red = float(start - end)
    div = red/start
    pct = 100 * div
    return round(pct, 3)


def printInfo(run_count, errs, ss_blocks, ss_threads, ss_ctxt, ss_pre, \
        end_blocks, end_threads, end_ctxt, end_pre):
    print "Number of Runs:", run_count
    print "Number of Different Errors Seen:", errs 

    ssb = getAvg(ss_blocks)
    sst = getAvg(ss_threads)
    ssc = getAvg(ss_ctxt)
    ssp = getAvg(ss_pre)
    eb = getAvg(end_blocks)
    et = getAvg(end_threads)
    ec = getAvg(end_ctxt)
    ep = getAvg(end_pre)

    
    print "Start Schedules:"
    print "\tAverage Scheduling Points:", ssb
    print "\tAverage Threads:", sst
    print "\tAverage Context Switches:", ssc
    print "\tAverage Preemptions:", ssp
    
    print "Simplified Schedules:"
    print "\tAverage Scheduling Points:", eb
    print "\tAverage Threads:", et
    print "\tAverage Context Switches:", ec
    print "\tAverage Preemptions:", ep
    
    print "Simplification (% reduction):"
    print "\tAverage Reduction in Scheduling Points:", getPctRed(ssb, eb), "%"
    print "\tAverage Reduction in Threads:", getPctRed(sst, et), "%"
    print "\tAverage Reduction in Context Switches:",getPctRed(ssc, ec), "%"
    print "\tAverage Reduction in Preemptions:",getPctRed(ssp, ep), "%"


def extractInt(id, item):
    assert id in item
    val = item.split(":")[1]
    val = val.strip()
    return int(val)

def main():
    if len(sys.argv) != 2:
        print "python generateSummaryStat.py [simpl-stat file]"
        sys.exit(1)

    statdir, statfile = os.path.split(sys.argv[1])

    fout = open(os.path.join(statdir, "simp-summarystat"), "w")

    assert os.path.exists(sys.argv[1])

    fin = open(sys.argv[1], "r").readlines()

    errdict = {}

    ss_blocks = []
    ss_threads = []
    ss_ctxt = []
    ss_pre = []

    end_blocks = []
    end_threads = []
    end_ctxt = []
    end_pre = []

    run_count = 0

    while len(fin) > 0:
        if "**RUN" in fin.pop(0):
            err = fin.pop(0)
            assert "Error:" in err
            errdict[err] = 1
            fin.pop(0)
            assert "Start Schedule:" in fin.pop(0)
    
            ss_blocks.append(extractInt("Total Blocks", fin.pop(0)))
            ss_threads.append(extractInt("Total Threads", fin.pop(0)))
            ss_ctxt.append(extractInt("Context Switches", fin.pop(0)))
            ss_pre.append(extractInt("Preemptions", fin.pop(0)))
            fin.pop(0)
            fin.pop(0)
            assert "Simplified Schedule" in fin.pop(0)
    
            end_blocks.append(extractInt("Total Blocks", fin.pop(0)))
            end_threads.append(extractInt("Total Threads", fin.pop(0)))
            end_ctxt.append(extractInt("Context Switches", fin.pop(0)))
            end_pre.append(extractInt("Preemptions", fin.pop(0)))

            run_count += 1

    printInfo(run_count, len(errdict.keys()), ss_blocks, ss_threads, \
            ss_ctxt, ss_pre, end_blocks, end_threads, end_ctxt, end_pre)

    sys.stdout = fout
    printInfo(run_count, len(errdict.keys()), ss_blocks, ss_threads, \
            ss_ctxt, ss_pre, end_blocks, end_threads, end_ctxt, end_pre)

    fout.close()


main()
