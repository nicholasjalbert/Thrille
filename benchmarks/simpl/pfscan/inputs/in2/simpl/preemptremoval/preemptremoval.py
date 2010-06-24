#preemptremoval.py - implements the preemption removal part of the 
# simplification algorithm
# "enable"
#B version coaleces all series of choices of the same thread and treats it 
#as one segment

import sys
import shutil
import os
import copy
tr = os.environ.get('THRILLE_ROOT')
if tr != None:
    sys.path.append(os.path.join(tr, "scripts/simpl/blockremoval"))
    import blockremoval

thrille_input_schedule = "thrille-sched"
thrille_output_schedule = "my-schedule"


def checkEnvironment():
    if len(sys.argv) < 4:
        print "usage: python preemptremoval.py [input schedule]",
        print "[output schedule] [binary under test] [binary flags]"
        print
        print "purpose: performs the preemption simplification algorithm",
        print "on the input schedule and outputs the results"
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert os.path.exists(sys.argv[1]), "input does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"

def countContextSwitches(sched, enabled, condlist):
    assert len(sched) == len(enabled)
    assert len(enabled) == len(condlist)
    assert len(sched) > 0
    total_cs = 0
    assert condlist[0] != "oncond"
    if (sched[0] != "0"): 
        total_cs += 1
    i = 1
    lastWasCond = False
    while i < len(sched):
        assert sched[i] in enabled[i], "schedule sanity fail"
        if (condlist[i] == "oncond"):
            lastWasCond = True
            i += 1
            continue
        if (lastWasCond):
            assert condlist[i] != "oncond"
            assert condlist[i-1] == "oncond"
            assert i-2 > 0
            if (sched[i] != sched[i-2]):
                total_cs += 1
            lastWasCond = False
        else:
            assert condlist[i] != "oncond"
            assert condlist[i-1] != "oncond"
            if (sched[i] != sched[i-1]):
                total_cs += 1
        i += 1
    return total_cs

def countNonpreemptiveCS(sched, enabled, condlist):
    assert len(sched) == len(enabled)
    assert len(enabled) == len(condlist)
    assert len(sched) > 0
    total_npcs = 0
    assert condlist[0] != "oncond"
    if (sched[0] != "0" and "0" not in enabled[0]):
        total_npcs += 1
    i = 1
    lastWasCond = False
    while i < len(sched):
        assert sched[i] in enabled[i], "schedule sanity fail"
        if (condlist[i] == "oncond"):
            lastWasCond = True
            i += 1
            continue
        if (lastWasCond):
            assert condlist[i] != "oncond"
            assert condlist[i-1] == "oncond"
            assert i-2 > 0
            if (sched[i] != sched[i-2]):
                if (sched[i-2] not in enabled[i]):
                    total_npcs += 1
            lastWasCond = False
        else:
            assert condlist[i] != "oncond"
            assert condlist[i-1] != "oncond"
            if (sched[i] != sched[i-1]):
                if (sched[i-1] not in enabled[i]):
                    total_npcs += 1
        i += 1
    return total_npcs


def countPreemptions(sched, enabled, condlist):
    assert len(sched) == len(enabled)
    assert len(enabled) == len(condlist)
    assert len(sched) > 0
    total_preempts = 0
    assert condlist[0] != "oncond"
    if (sched[0] != "0" and "0" in enabled[0]):
        total_preempts += 1
    i = 1
    lastWasCond = False
    while i < len(sched):
        assert sched[i] in enabled[i], "schedule sanity fail"
        if (condlist[i] == "oncond"):
            lastWasCond = True
            i += 1
            continue
        if (lastWasCond):
            assert condlist[i] != "oncond"
            assert condlist[i-1] == "oncond"
            assert i-2 > 0
            if (sched[i] != sched[i-2]):
                if (sched[i-2] in enabled[i]):
                    total_preempts += 1
            lastWasCond = False
        else:
            assert condlist[i] != "oncond"
            assert condlist[i-1] != "oncond"
            if (sched[i] != sched[i-1]):
                if (sched[i-1] in enabled[i]):
                    total_preempts += 1
        i += 1
    return total_preempts


#non-destructively moves the target (and all following choices that
#are the same thread) to directly after the frontier
def moveTargetUnitAfterFrontier(schedule, minimal_frontier, target):
    assert target < len(schedule)
    assert minimal_frontier < len(schedule)
    assert minimal_frontier < target
    newsched = copy.deepcopy(schedule)
    my_item = newsched.pop(target)
    assert my_item == newsched[minimal_frontier], "item fail"
    while True: 
        newsched.insert(minimal_frontier + 1, my_item)
        target += 1
        if target >= len(newsched):
            break
        if newsched[target] != newsched[minimal_frontier]:
            break
        my_item = newsched.pop(target)
        assert my_item == newsched[minimal_frontier], "item fail"
    return newsched

#moves on choice point to directly after the frontier
def moveSwitchAfterFrontier(schedule, minimal_frontier, target):
    assert target < len(schedule)
    assert minimal_frontier < len(schedule)
    assert minimal_frontier < target
    newsched = copy.deepcopy(schedule)
    item = newsched.pop(target)
    assert item == newsched[minimal_frontier]
    newsched.insert(minimal_frontier + 1, item)
    return newsched

#given a schedule, it reads and updates enabled and condlist
def regenerateEnabled(binary_file, schedule, addrlist):
    enabled = []
    newsched = []
    condlist = []
    binpath, binfile = os.path.split(binary_file)
    fin = os.path.join(binpath, thrille_output_schedule)
    assert os.path.exists(fin), "Can't find output schedule"
    schedin = open(fin, "r").readlines()
    if "begin_addr_list" in schedin[0]:
        while True:
            if "end_addr_list" in schedin.pop(0):
                break
    i = 0
    while i < len(schedin):
        if "assert" in schedin[i]:
            break
        assert schedin[i].strip() == schedule[i/2], "Schedule synch error"
        newsched.append(schedin[i].strip())
        i += 1
        tmp = schedin[i].split(":")
        assert len(tmp) == 2, "Schedule format error"
        assert (tmp[0] == "enabled" or tmp[0] == "oncond")
        condlist.append(tmp[0])
        tmp = tmp[1]
        tmp = tmp.strip()
        tmp = tmp.strip(",")
        enabled_threads = tmp.split(",")
        enabled.append(enabled_threads)
        i += 1
    print len(schedule)
    print len(enabled)
    if len(schedule) != len(enabled):
        blockremoval.outputNewSchedule(schedule, addrlist, os.path.join(binpath ,"failoldsched"));
        blockremoval.outputNewSchedule(newsched, addrlist, os.path.join(binpath , "failnewsched"));
    assert len(schedule) == len(enabled)
    assert len(condlist) == len(schedule)
    return enabled, condlist


#no coalescing
def preemptRemovalAlgorithmA(binary_file, output_schedule, thrille_root, \
        sched, enabled, addrlist, condlist, error):
    start_preemptions = countPreemptions(sched, enabled, condlist)
    minimal_frontier = 0
    number_success = 0
    while minimal_frontier < len(sched):
        curr_thread = sched[minimal_frontier]
        target = minimal_frontier + 1
        if target < len(sched):
            if curr_thread == sched[target]:
                minimal_frontier += 1
                continue
            if curr_thread not in enabled[target]:
                minimal_frontier += 1
                continue
        while target < len(sched):
            if sched[target] == curr_thread:
                newsched = moveTargetUnitAfterFrontier(sched, \
                        minimal_frontier, target)
                assert len(newsched) == len(sched)
                success = blockremoval.testSchedule(newsched, error, addrlist,\
                        binary_file, thrille_root)
                if (success):
                    old_sched_len = len(sched)
                    binpath, binfile = os.path.split(binary_file)
                    fin = os.path.join(binpath, thrille_output_schedule)
                    sched, enabled, addrlist, condlist = \
                            blockremoval.readInNewSchedule(fin)
                    if ("segfault" not in error): 
                        assert old_sched_len == len(sched)
                    else:
                        if len(sched) != old_sched_len:
                            print "Segfault weirdness"
                            print "old schedule length:", old_sched_len
                            print "new schedule length:", len(sched)
                    print "Preemptions at start:", start_preemptions
                    print "Preemptions now:", countPreemptions(sched, \
                            enabled, condlist)
                    print "Frontier at", minimal_frontier, "out of", 
                    print len(sched)
                    print "Number of swaps:", number_success
                    number_success += 1
                    if number_success % 50 == 0:
                        print "***OUTPUT***"
                        blockremoval.outputResult(sched, error, addrlist, \
                                binary_file, thrille_root, output_schedule)
                    break
                else:
                    print "Preemptions at start:", start_preemptions
                    print "Preemptions now:", countPreemptions(sched, \
                            enabled, condlist)
                    print "Frontier at", minimal_frontier, "out of",
                    print len(sched)
                    print "Number of swaps:", number_success
                    break
            else:
                target += 1
        minimal_frontier += 1
    assert blockremoval.testSchedule(sched, error, addrlist, binary_file, \
            thrille_root), "sanity fail"
    return sched

#coalescing
def preemptRemovalAlgorithmB(binary_file, output_schedule, thrille_root, \
        sched, enabled, addrlist, condlist, error):
    start_preemptions = countPreemptions(sched, enabled, condlist)
    minimal_frontier = 0
    number_success = 0
    while minimal_frontier < len(sched):
        curr_thread = sched[minimal_frontier]
        target = minimal_frontier + 1
        if target < len(sched):
            if curr_thread == sched[target]:
                minimal_frontier += 1
                continue
            if curr_thread not in enabled[target]:
                minimal_frontier += 1
                continue
        while target < len(sched):
            if sched[target] == curr_thread:
                newsched = moveSwitchAfterFrontier(sched, \
                        minimal_frontier, target)
                success = blockremoval.testSchedule(newsched, error, addrlist,\
                        binary_file, thrille_root)
                if (success):
                    old_sched_len = len(sched)
                    binpath, binfile = os.path.split(binary_file)
                    fin = os.path.join(binpath, thrille_output_schedule)
                    sched, enabled, addrlist, condlist = \
                            blockremoval.readInNewSchedule(fin)
                    if ("segfault" not in error):
                        assert old_sched_len == len(sched)
                    else:
                        if len(sched) != old_sched_len:
                            print "Segfault weirdness"
                            print "old schedule length:", old_sched_len
                            print "new schedule length:", len(sched)

                    print "Preemptions at start:", start_preemptions
                    print "Preemptions now:", countPreemptions(sched, \
                            enabled, condlist)
                    print "Frontier at", minimal_frontier, "out of", 
                    print len(sched)
                    print "Number of swaps:", number_success
                    number_success += 1
                    if number_success % 50 == 0:
                        print "***OUTPUT***"
                        blockremoval.outputResult(sched, error, addrlist, \
                                binary_file, thrille_root, output_schedule)
                    break
                else:
                    print "Preemptions at start:", start_preemptions
                    print "Preemptions now:", countPreemptions(sched, \
                            enabled, condlist)
                    print "Frontier at", minimal_frontier, "out of",
                    print len(sched)
                    print "Number of swaps:", number_success
                    break
            else:
                target += 1
        minimal_frontier += 1
    assert blockremoval.testSchedule(sched, error, addrlist, binary_file, \
            thrille_root), "sanity fail"
    return sched

def testCountContextSwitches():
    i = []
    en = []
    c = []
    try:
        countContextSwitches(i, en, c)
    except AssertionError:
        pass
    else:
        assert False    

    i = ["1"]
    en = []
    c = []
    try:
        countContextSwitches(i, en, c)
    except AssertionError:
        pass
    else:
        assert False

    i = ["0"]
    en = [["0"]]
    c = ["enabled"]
    assert countContextSwitches(i, en, c) == 0

    i = ["1"]
    en = [["0", "1"]]
    c = ["enabled"]
    assert countContextSwitches(i, en, c) == 1


    i = ["1", "2"]
    en = [["0"], ["1"]]
    c = ["enabled", "enabled"]
    try:
        countContextSwitches(i, en, c)
    except:
        pass
    else:
        assert False

    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','2'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['6','3'],['3'],['1','2','3']]
    c = ["enabled","enabled","enabled","enabled","enabled","enabled", \
            "enabled","enabled","enabled","enabled","enabled"]
    assert countContextSwitches(i, en, c) == 8
    
    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','7'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['3', '4'],['3'],['1','2','3']]
    c = ["enabled","enabled","oncond","enabled","enabled","enabled", \
            "enabled","enabled","oncond","enabled","enabled"]
    assert countContextSwitches(i, en, c) == 7 

    
    return 1


def testCountNonpreemptiveCS():
    i = []
    en = []
    c = []
    try:
        countNonpreemptiveCS(i, en, c)
    except AssertionError:
        pass
    else:
        assert False    

    i = ["1"]
    en = []
    c = []
    try:
        countNonpreemptiveCS(i, en, c)
    except AssertionError:
        pass
    else:
        assert False

    i = ["0"]
    en = [["0"]]
    c = ["enabled"]
    assert countNonpreemptiveCS(i, en, c) == 0

    i = ["1"]
    en = [["0", "1"]]
    c = ["enabled"]
    assert countNonpreemptiveCS(i, en, c) == 0


    i = ["1", "2"]
    en = [["0"], ["1"]]
    c = ["enabled", "enabled"]
    try:
        countNonpreemptiveCS(i, en, c)
    except:
        pass
    else:
        assert False

    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','2'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['6','3'],['3'],['1','2','3']]
    c = ["enabled","enabled","enabled","enabled","enabled","enabled", \
            "enabled","enabled","enabled","enabled","enabled"]
    assert countNonpreemptiveCS(i, en, c) == 4  
    

    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','7'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['3', '4'],['3'],['1','2','3']]
    c = ["enabled","enabled","oncond","enabled","enabled","enabled", \
            "enabled","enabled","oncond","enabled","enabled"]
    assert countNonpreemptiveCS(i, en, c) == 5 
    return 1


def testCountPreemptions():
    i = []
    en = []
    c = []
    try:
        countPreemptions(i, en, c)
    except AssertionError:
        pass
    else:
        assert False    

    i = ["1"]
    en = []
    c = []
    try:
        countPreemptions(i, en, c)
    except AssertionError:
        pass
    else:
        assert False

    i = ["0"]
    en = [["0"]]
    c = ["enabled"]
    assert countPreemptions(i, en, c) == 0

    i = ["1"]
    en = [["0", "1"]]
    c = ["enabled"]
    assert countPreemptions(i, en, c) == 1


    i = ["1", "2"]
    en = [["0"], ["1"]]
    c = ["enabled", "enabled"]
    try:
        countPreemptions(i, en, c)
    except:
        pass
    else:
        assert False

    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','2'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['6','3'],['3'],['1','2','3']]
    c = ["enabled","enabled","enabled","enabled","enabled","enabled", \
            "enabled","enabled","enabled","enabled","enabled"]
    assert countPreemptions(i, en, c) == 4  
    

    i = ['1','2','0','4','5','6','6','6','3','3','1']
    en = [['1'],['2'],['0','7'],['4'],['5','4'],['6'],['6','2'],['6','1'],\
            ['3', '4'],['3'],['1','2','3']]
    c = ["enabled","enabled","oncond","enabled","enabled","enabled", \
            "enabled","enabled","oncond","enabled","enabled"]
    assert countPreemptions(i, en, c) == 2 
    return 1


def testMoveTargetUnitAfterFrontier():
    i = [1,2,3]
    try:
       moveTargetUnitAfterFrontier(i, 0, 2)
    except:
       pass
    else:
       assert False 
    
    i = [1,2,3,3,4,3,3,3,6,2]
    assert moveTargetUnitAfterFrontier(i, 1, 9) == [1,2,2,3,3,4,3,3,3,6]
    assert moveTargetUnitAfterFrontier(i, 3, 5) == [1,2,3,3,3,3,3,4,6,2]
    return 1 
    
def testMoveSwitchAfterFrontier():
    i = [1,2,3,4]
    try:
        moveSwitchAfterFrontier(i, 0, 3)
    except:
        pass
    else:
        assert False

    i = [1,1,3,4,1]
    assert moveSwitchAfterFrontier(i, 1, 4) == [1,1,1,3,4]
    i = [1,1,3,4,1,1]
    assert moveSwitchAfterFrontier(i, 1, 4) == [1,1,1,3,4,1]
    return 1 
   
def testRegenerateEnabled():
    try:
        regenerateEnabled("./binaryfile", ["1", "2", "3", "1"], [])
    except:
        pass
    else:
        assert False
    
    fout = open(thrille_output_schedule, "w")
    fout.write("1\nenabled:1,2,3,\n2\nenabled:1,2,3,\n3\noncond:1,2,3,\n")
    fout.write("1\nenabled:1,2,3,\nassert false\n")
    fout.close()
    myen, myc = regenerateEnabled("./binaryfile", ["1", "2", "3", "1"], [])
    assert myen == [["1","2","3"],["1","2","3"],["1","2","3"],["1","2","3"]]
    assert myc == ["enabled", "enabled", "oncond", "enabled"]

    fout = open(thrille_output_schedule, "w")
    fout.write("2\nenabled:1,2,3,\n2\nenabled:1,2,3,\n3\nenabled:1,2,3,\n")
    fout.write("1\nenabled:1,2,3,\nassert false\n")
    fout.close()
    try:
        regenerateEnabled("./binaryfile", ["1", "2", "3", "1"], [])
    except:
        pass
    else:
        assert False

    fout = open(thrille_output_schedule, "w")
    fout.write("1\nenabled1,2,3,\n2\nenabled:1,2,3,\n3\nenabled:1,2,3,\n")
    fout.write("1\nenabled:1,2,3,\nassert false\n")
    fout.close()
    try:
        regenerateEnabled("./binaryfile", ["1", "2", "3", "1"], [])
    except:
        pass
    else:
        assert False

    os.remove(thrille_output_schedule) 
    return 1 
  
def testPreemptRemovalAlgorithmA():
    return 0
 
def testPreemptRemovalAlgorithmB():
    return 0 

def test():
    total_pass = 0
    total_pass += testCountContextSwitches()
    total_pass += testCountNonpreemptiveCS()
    total_pass += testCountPreemptions()
    total_pass += testMoveTargetUnitAfterFrontier()
    total_pass += testMoveSwitchAfterFrontier()
    try:
        total_pass += testRegenerateEnabled()
    except:
        os.remove(thrille_output_schedule)
        raise
    total_pass += testPreemptRemovalAlgorithmA()
    total_pass += testPreemptRemovalAlgorithmB()
    print "Total Tests Passed:", total_pass



    





if __name__ == "__main__":
    test()



