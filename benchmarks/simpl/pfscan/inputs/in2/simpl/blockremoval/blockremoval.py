#blockremoval.py -- implements several variants of the block removal algorithm
# to run unit tests, execute directly
# use as a library other scripts which implement the whole algorithm
#
#algorithm version A - treats each choice at a schedule point as a segment
#version B - coalesce all series of same choices as one segment
#version C - experimental, not working correctly, but want to iterate between
#  the extremes of versions A and B

import os
import sys
import copy
import shutil

thrille_input_schedule = "thrille-sched"
thrille_output_schedule = "my-schedule"

class SignalPoint:
    def __init__(self, _chosen, _caller, _isbroad, _addr, _cond, _oncond):
        self.chosen = _chosen
        self.caller = _caller
        self.isbroad = _isbroad
        self.addr = _addr
        self.cond = _cond
        self.oncond = _oncond

class SchedulePoint:
    def __init__(self, _chosen, _caller, _type, _addr, _enabled):
        self.chosen = _chosen
        self.caller = _caller
        self.type = _type
        self.addr = _addr
        self._enabled = enabled


#reads the my-schedule file that thrille outputs 
#and returns a list of schedule choices and a list of lists
# of threads enabled at each schedule point

#addrlist - the iid of the memory accesses we treat as sched points
#condlist - whether or not each step in the schedule represent
#           waking up a thread waiting on a condition
def readInNewSchedule(inputfile):
    assert os.path.exists(inputfile), "Input file does not exist"
    addrlist = []
    schedule = []
    enabled = []
    condlist = []
    schedin = open(inputfile, "r").readlines()
    if "begin_addr_list" in schedin[0]:
        while True:
            item = schedin.pop(0)
            addrlist.append(item.strip())
            if "end_addr_list" in item:
                break

    while True:
        if len(schedin) <= 0:
            break
        item = schedin.pop(0).strip()
        if "assert" in item or "end_log" in item:
            break
        schedule.append(item)
        tmp = schedin.pop(0).split(":")
        assert len(tmp) == 2, "Schedule format error"
        assert (tmp[0] == "enabled" or tmp[0] == "oncond")
        condlist.append(tmp[0])
        tmp = tmp[1]
        tmp = tmp.strip()
        tmp = tmp.strip(",")
        enabled_threads = tmp.split(",")
        assert item in enabled_threads, "schedule sanity fail"
        enabled.append(enabled_threads)

    return schedule, enabled, addrlist, condlist

#returns the textual error in the schedule file --
#expected to be of the form of an assert violation
def recordFailure(inputfile):
    assert os.path.exists(inputfile), "Input file does not exist"
    schedule = []
    schedin = open(inputfile, "r").readlines()
    segfaultcheck = schedin[-2].strip()
    assert len(schedin) > 0
    while True:
        item = schedin.pop(0)
        if "assert" in item:
            return item
        if "end_log" in item:
            assert len(schedin) == 0
            break
        if len(schedin) <= 0:
            return "segfault" + segfaultcheck
    return None


#returns a dictionary to track whether the algorithm
#is done with each thread
def initializeDoneMap(schedule):
    assert len(schedule) > 0
    done_map = {}
    for schedblock in schedule:
        done_map[schedblock] = False

    done_map[schedule[-1]] = True
    return done_map


def getTotalBlocks(schedule):
    return len(schedule)

def algorithmDone(done_map):
    iamdone = True 
    for x in done_map:
        iamdone = iamdone and done_map[x]
    return iamdone

#non-destructively removes the last (singular) block of a thread
#that the algorithm hasn't finished with
def removeThreadBlock(done_map, sched):
    newsched = copy.deepcopy(sched)
    i = -1
    while abs(i) <= len(newsched):
        curr_block = sched[i]
        if not done_map[curr_block]:
            print "Removing the", i, "scheduling block"
            tid = newsched.pop(i)
            return newsched, tid
        i -= 1

    return newsched, None

def outputNewSchedule(schedule, addrlist, outname):
    fout = open(outname, "w")
    assert "begin_addr_list" in addrlist[0]
    assert "end_addr_list" in addrlist[-1]
    for x in addrlist:
        fout.write(x + "\n")
    for x in schedule:
        fout.write(x + "\n")
        fout.write("python_generated_schedule\n")
    fout.close()

#execute the thriller
def executeSchedule(thrille_root, binary, binflags):
    os.environ['LD_PRELOAD'] =  \
            os.path.join(thrille_root, "bin", "libstrictserial.so")
    binarydir, bin = os.path.split(binary)
    binflags.insert(0, bin)
    print "Spawning:"
    print "\t", binary
    print "\t", binflags
    thing = os.spawnve(os.P_NOWAIT, binary, binflags, os.environ)
    pid, exit = os.waitpid(thing, 0)
    del os.environ['LD_PRELOAD']

def getBinaryFlags():
    i = 4
    binflags = []
    while i < len(sys.argv):
        binflags.append(sys.argv[i])
        i += 1
    return binflags

#given a schedule, does the necessary output etc
#and executes the schedule.  Determine whether or not
#the given error manifests
def testSchedule(schedule, error, addrlist, binary, thrille_root):
    binarydir, bin = os.path.split(binary)
    curr_dir = os.getcwd()
    if binarydir != '':
        os.chdir(binarydir)
    binflags = getBinaryFlags()
    outputNewSchedule(schedule, addrlist, thrille_input_schedule)
    executeSchedule(thrille_root, binary, binflags)
    new_error = recordFailure(os.path.join(os.getcwd(), \
            thrille_output_schedule))
    os.chdir(curr_dir)
    print "target error:", error
    print "got error:", new_error
    if (error == new_error):
        return True
    return False

#ensures the output schedule manifests the error, then outputs it
def outputResult(schedule, error, addrlist, binary, thrille_root, out):
    assert testSchedule(schedule, error, addrlist, \
            binary, thrille_root), "fail"
    binarydir, bin = os.path.split(binary)
    shutil.copy(os.path.join(binarydir, thrille_output_schedule), out);

def checkEnvironment():
    if len(sys.argv) < 4:
        print "usage: python blockremoval.py [input schedule]",
        print "[output schedule] [binary under test] [binary flags]"
        print
        print "purpose: performs the block removal schedule simplfication",
        print "schedule on an input schedule and outputs the resulting",
        print "schedule"
        sys.exit(1)
    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert os.path.exists(sys.argv[1]), "input does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"

#coalesces a series of segments executed by the same thread.
#returns a the schedule divided into three segments:
#[start] ['removed' segment of one thread] [end] 
#if you concat these lists togehter, you get the original schedule
def removeThreadSegment(done_map, sched):
    assert done_map[sched[-1]], "Done map incorrect"
    i = -1
    while abs(i) <= len(sched):
        curr_block = sched[i]
        if not done_map[curr_block]:
            target_end = i + 1
            while sched[i] == curr_block:
                i -= 1
                if abs(i) > len(sched):
                    break
            target_begin = i + 1
            start_list = sched[:target_begin]
            mid_list = sched[target_begin:target_end]
            end_list = sched[target_end:]
            return start_list, mid_list, end_list 
        i -= 1

    return [], [], sched 


#treats only individual thread segments at a time
def blockRemovalAlgorithmA(binary_file, thrille_root, sched, \
        enabled, addrlist, error):
    start_blocks = getTotalBlocks(sched)
    done_map = initializeDoneMap(sched)
    blocksRemoved = 0
    while not algorithmDone(done_map):
        newsched, tid = removeThreadBlock(done_map, sched)
        success = testSchedule(newsched, error, addrlist, \
                binary_file, thrille_root)
        if not success or tid == None:
            print "thread", tid, "is done"
            if tid == None:
                break
            else:
                done_map[tid] = True
        else:
            print "thread", tid, "block removed"
            blocksRemoved += 1
            sched = newsched
    end_blocks = getTotalBlocks(sched)
    assert start_blocks - end_blocks == blocksRemoved, "Block consistency fail"
    assert testSchedule(sched, error, addrlist, \
            binary_file, thrille_root), "sched fail"
    return sched

#coalesces individual thread segments into a single unit
def blockRemovalAlgorithmB(binary_file, thrille_root, sched, enabled, \
        addrlist, error):
    start_blocks = getTotalBlocks(sched)
    done_map = initializeDoneMap(sched)
    blocksRemoved = 0
    while not algorithmDone(done_map):
        start_s, mid_s, end_s= removeThreadSegment(done_map, sched)
        if len(mid_s) == 0:
            break
        my_removal = mid_s[0]
        for x in mid_s:
            assert x == my_removal, "removal sanity fail"
        newsched = start_s + end_s
        success = testSchedule(newsched, error, addrlist, binary_file, \
                thrille_root)
        if not success:
            print "thread", my_removal, "is done"
            done_map[my_removal] = True
        else:
            print "thread", my_removal, "block removed"
            blocksRemoved += len(mid_s)
            sched = newsched
    end_blocks = getTotalBlocks(sched)
    assert start_blocks - end_blocks == blocksRemoved, "Block consistency fail"
    assert testSchedule(sched, error, addrlist, \
            binary_file, thrille_root), "sched fail"
    return sched

#attempts to coalesce, and slowly adds back in if fail
def blockRemovalAlgorithmC(binary_file, thrille_root, sched, enabled, \
        addrlist, error):
    start_blocks = getTotalBlocks(sched)
    done_map = initializeDoneMap(sched)
    blocksRemoved = 0
    while not algorithmDone(done_map):
        my_midsection = [] 
        newsched = []
        start_s, mid_s, end_s= removeThreadSegment(done_map, sched)
        if len(mid_s) == 0:
            break
        my_removal = mid_s[0]
        for x in mid_s:
            assert x == my_removal, "removal sanity fail"
        success = False
        while not success and len(mid_s) > 0:
            newsched = start_s + my_midsection + end_s
            success = testSchedule(newsched, error, addrlist, \
                    binary_file, thrille_root)
            my_midsection.append(mid_s.pop(0))
            if not success:
                print "thread", my_removal, "removal section of", len(mid_s), 
                print "fail"
            else:
                print "thread", my_removal, "removal of", len(mid_s), \
                        "SUCCESS"

        if not success:
            print "thread", my_removal, "is done"
            done_map[my_removal] = True
        else:
            blocksRemoved += len(mid_s)
            sched = newsched
    end_blocks = getTotalBlocks(sched)
    print start_blocks
    print end_blocks
    print blocksRemoved
    assert start_blocks - end_blocks == blocksRemoved, "Block consistency fail"
    assert testSchedule(sched, error, addrlist, binary_file, thrille_root), \
            "sched fail"
    return sched

def main():
    checkEnvironment()
    input_schedule = sys.argv[1]
    assert "my-schedule" not in input_schedule, "Input schedule err"
    assert "thrille-sched" not in input_schedule, "Input schedule err"
    output_schedule = sys.argv[2]
    assert "my-schedule" not in output_schedule, "Output schedule err"
    assert "thrille-sched" not in output_schedule, "Output schedule err"
    binary_file = sys.argv[3]
    thrille_root = os.environ.get('THRILLE_ROOT')
    sched, enabled, addrlist, condlist = readInNewSchedule(input_schedule)
    error = recordFailure(input_schedule)
    totalBlocks = getTotalBlocks(sched)
    
    sched = blockRemovalAlgorithmA( \
            binary_file, thrille_root, sched, enabled, addrlist, error);

    outputResult(sched, error, addrlist, binary_file, thrille_root, \
            output_schedule)
    newBlocks = getTotalBlocks(sched)
    blocksRemoved = totalBlocks - newBlocks
    print "Schedule Block Removal Algorithm Done"
    print "Total Blocks:", totalBlocks
    print "Blocks Removed:", blocksRemoved
    print "Blocks Remaining:", getTotalBlocks(sched)

def testReadInNewSchedule():
    try:
        readInNewSchedule("blahsched")
    except:
        pass
    else:
        assert False

    fout = open("blahsched", "w")
    fout.write("1\nenabled\nassert\n")
    fout.close()
    try:
        readInNewSchedule("blahsched")
    except:
        pass
    else:
        assert False

    fout = open("blahsched", "w")
    fout.write("0\nenabled:0,1,\n1\nenabled:0,1,\n2\noncond:1,2,\nassert\n")
    fout.close()
    choice, enabled, addr, condlist = readInNewSchedule("blahsched")
    assert choice == ['0', '1', '2']
    assert enabled == [['0','1'], ['0','1'], ['1','2']]
    assert addr == []
    assert condlist == ["enabled", "enabled", "oncond"]

    fout = open("blahsched", "w")
    fout.write("begin_addr_list\n0x100\n0x300\nend_addr_list\n0\n")
    fout.write("enabled:0,1,\n1\nenabled:0,1,\n2\nenabled:1,2,\nend_log\n")
    fout.close()
    choice, enabled, addr, condlist = readInNewSchedule("blahsched")
    assert choice == ['0', '1', '2']
    assert enabled == [['0','1'], ['0','1'], ['1','2']]
    assert addr == ["begin_addr_list","0x100","0x300","end_addr_list"]
    assert condlist == ["enabled", "enabled", "enabled"]

    fout = open("blahsched", "w")
    fout.write("0\nenabled:2,1,\n1\nenabled:0,1,\n2\nenabled:1,2,\nassert\n")
    fout.close()
    try:
        choice, enabled, addr, condlist = readInNewSchedule("blahsched")
    except:
        pass
    else:
        assert False

    fout = open("blahsched", "w")
    fout.write("assert\n")
    fout.close()
    choice, enabled, addr, condlist = readInNewSchedule("blahsched")
    assert choice == []
    assert enabled == []
    assert addr == []
    assert condlist == []

    fout = open("blahsched", "w")
    fout.write("0\nenabled:0,1,\n1\nenabled:0,1,\n2\nenabled:1,2,\n")
    fout.close()
    choice, enabled, addr, condlist = readInNewSchedule("blahsched")
    assert choice == ["0", "1", "2"]
    assert enabled == [['0','1'], ['0','1'], ['1','2']]
    assert addr == []
    assert condlist == ["enabled", "enabled", "enabled"]

    os.remove('blahsched')
    return 1

def testRecordFailure():
    try:
        recordFailure("blahsched")
    except:
        pass
    else:
        assert False

    fout = open("blahsched", "w")
    fout.write("1\nenabled:1,\nassert die_error\n")
    fout.close()

    assert recordFailure("blahsched") == "assert die_error\n"

    fout = open("blahsched", "w")
    fout.write("1\nenabled:1,\n")
    fout.close()

    assert recordFailure("blahsched") == "segfault1" 

    fout = open("blahsched", "w")
    fout.write("1\nenabled:1,\n2\nenabled:1,2,\n")
    fout.close()

    assert recordFailure("blahsched") == "segfault2" 



    fout = open("blahsched", "w")
    fout.write("1\nenabled:1,\nend_log\n")
    fout.close()

    assert recordFailure("blahsched") == None

    return 1

def testInitializeDoneMap():
    i = []
    try:
        initializeDoneMap(i)
    except:
        pass
    else:
        assert False
    i = [1]
    assert initializeDoneMap(i) == {1:True}
    i = [1,2,2,2,4,3,2,4]
    assert initializeDoneMap(i) == {1:False, 2:False, 3:False, 4:True}
    return 1

def testGetTotalBlocks():
    i = []
    assert getTotalBlocks(i) == 0
    i = [1,2,3]
    assert getTotalBlocks(i) == 3
    return 1

def testAlgorithmDone():
    d = {}
    assert algorithmDone(d)
    d = {1:True, 2:True}
    assert algorithmDone(d)
    d = {1:False, 2:True, 3:True}
    assert not algorithmDone(d)
    return 1

def testRemoveThreadBlock():
    d = {1:True, 2:False, 3:True}
    i = [1,2,2,3,2,2,1]
    new, tid = removeThreadBlock(d, i)
    assert tid == 2
    assert new == [1,2,2,3,2,1]

    d = {1:True, 2:True, 3:True}
    new, tid = removeThreadBlock(d, i)
    assert tid == None
    assert new == [1,2,2,3,2,2,1]

    d = {1:True, 2:False, 3:True}
    i = [1,3,1]
    new, tid = removeThreadBlock(d, i)
    assert tid == None
    assert new == [1,3,1]

    d = {1:True, 2:False, 3:True}
    i = [2,1,3,1]
    new, tid = removeThreadBlock(d, i)
    assert tid == 2 
    assert new == [1,3,1]
    return 1

def testOutputNewSchedule():
    outputNewSchedule(['1'], ["begin_addr_list", "end_addr_list"], "blahsched")
    fin = open("blahsched", "r").readlines()
    assert fin == ["begin_addr_list\n", "end_addr_list\n", "1\n", \
            "python_generated_schedule\n"]
    os.remove("blahsched")
    return 1

def testExecuteSchedule():
    print "TODO: implement executeSchedule test"
    return 0

def testGetBinaryFlags():
    a = [1,2,3,4,5,6]
    tmp = sys.argv
    sys.argv = a
    assert getBinaryFlags() == [5,6]
    sys.argv = [1,2,3,4]
    assert getBinaryFlags() == []
    sys.argv = tmp
    return 1

def testTestSchedule():
    print "TODO: implement testSchedule test"
    return 0

def testOutputResult():
    print "TODO: implement outputResult test"
    return 0

def testRemoveThreadSegment():
    d = {1:True, 2:False, 3:True}
    i = [1,2,2,3,2,2,1]
    start, mid, end = removeThreadSegment(d, i)
    assert start == [1,2,2,3]
    assert mid == [2,2]
    assert end == [1]

    d = {1:False, 2:True, 3:True}
    i = [1,2,2,3,2,2,1]
    try:
        start, mid, end = removeThreadSegment(d, i)
    except:
        pass
    else:
        assert False


    d = {1:True, 2:True, 3:True}
    start, mid, end = removeThreadSegment(d, i)
    assert start == [] 
    assert mid == []
    assert end == [1,2,2,3,2,2,1]

    d = {1:True, 2:False, 3:True}
    i = [2,2,2,1,3,1]
    start, mid, end = removeThreadSegment(d, i)
    assert start == []
    assert mid == [2,2,2]
    assert end == [1,3,1]


    d = {1:True, 2:False, 3:True}
    i = [1,3,1]
    start, mid, end = removeThreadSegment(d, i)
    assert start == []
    assert mid == []
    assert end == [1,3,1]

    d = {1:True, 2:False, 3:True}
    i = [1,3,2,1,3,1]
    start, mid, end = removeThreadSegment(d, i)
    assert start == [1,3]
    assert mid == [2]
    assert end == [1,3,1]

    return 1

def test():
    total_passed = 0
    try:
        total_passed += testReadInNewSchedule()
    except:
        os.remove("blahsched")
        raise
    try:
        total_passed += testRecordFailure()
    except:
        os.remove("blahsched")
        raise
    total_passed += testInitializeDoneMap()
    total_passed += testGetTotalBlocks()
    total_passed += testAlgorithmDone()
    total_passed += testRemoveThreadBlock()
    total_passed += testOutputNewSchedule()
    total_passed += testExecuteSchedule()
    total_passed += testGetBinaryFlags()
    total_passed += testTestSchedule()
    total_passed += testOutputResult()
    total_passed += testRemoveThreadSegment()
    print "Tests Passed:", total_passed


if __name__ == "__main__":
    test()
    
