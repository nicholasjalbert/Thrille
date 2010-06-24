# object oriented implementation of Schedule and SchedulePoint for
# trace simplification algorithm
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal Matter>

import os
import sys
import copy
import subprocess
import shutil

class SchedulePoint:

    def __init__(self, list):
        assert len(list) >= 0
        if "signalled" in list[0]:
            assert len(list) == 11
            assert "signalled" in list[0]
            assert "caller" in list[1]
            assert "idaddr" in list[2]
            assert "cond" in list[3]
            assert "oncond" in list[4]
            assert "SCHED" in list[5]
            assert "chosen" in list[6]
            assert "caller" in list[7]
            assert "typstr" in list[8]
            assert "Simulate_Signal" in list[8]
            assert "idaddr" in list[9]
            assert "enable" in list[10]
            self.chosen = self.splitColonSeparatedInput(list[6])
            self.caller = self.splitColonSeparatedInput(list[7])
            self.type = self.splitColonSeparatedInput(list[8])
            self.addr = self.splitColonSeparatedInput(list[9])
            self.enabled = self.splitCommaSeparatedInput(list[10])
            self.signalled = self.splitColonSeparatedInput(list[0])
            sanitycheck = self.splitColonSeparatedInput(list[1])
            self.cond = self.splitColonSeparatedInput(list[3])
            self.oncond = self.splitCommaSeparatedInput(list[4])
            assert sanitycheck == self.caller
        else:
            assert len(list) == 5
            assert "chosen" in list[0]
            assert "caller" in list[1]
            assert "typstr" in list[2]
            assert "idaddr" in list[3]
            assert "enable" in list[4]
            self.chosen = self.splitColonSeparatedInput(list[0])
            self.caller = self.splitColonSeparatedInput(list[1])
            self.type = self.splitColonSeparatedInput(list[2])
            self.addr = self.splitColonSeparatedInput(list[3])
            self.enabled = self.splitCommaSeparatedInput(list[4])

    def __repr__(self):
        string = "SchedulePoint(" + "chosen: " + self.chosen + ", "
        string += "caller: " + self.caller + ", "
        string += "type: " + self.type + ", "
        string += "addr: " + self.addr + ", "
        string += "enabled: " + str(self.enabled) 

        if hasattr(self, "signalled"):
            string += ", signalled: " + self.signalled + ", "
            string += "cond: " + self.cond + ", "
            string += "oncond: " + str(self.oncond)

        string += ")"
        return string

    def __eq__(self, other):
        if not isinstance(other, SchedulePoint):
            return False
        for x in dir(self):
            if not hasattr(other, x):
                return False
        if self.chosen != other.chosen:
            return False
        if self.caller != other.caller:
            return False
        if self.type != other.type:
            return False
        if self.addr != other.addr:
            return False
        if self.enabled != other.enabled:
            return False
        if hasattr(self, "signalled"):
            if self.signalled != other.signalled:
                return False
            if self.oncond != other.oncond:
                return False
        return True

    def __ne__(self, other):
        return not (self.__eq__(other))

    def splitColonSeparatedInput(self, input):
        tmp = input.split(":")
        assert len(tmp) == 2
        tmp = tmp[1].strip()
        return tmp

    def splitCommaSeparatedInput(self, input):
        tmp = input.split(":")
        assert len(tmp) == 2
        tmp = tmp[1].strip()
        tmp = tmp.strip(",")
        tmp = tmp.split(",")
        assert len(tmp) > 0
        return tmp



class Schedule:


    def __init__(self, file_in = None):
        self.schedule = []
        self.addrlist = []
        self.error = None
        if (file_in is not None):
            self.parseFile(file_in)

    def cloneThyself(self):
        k = Schedule()
        k.schedule = copy.deepcopy(self.schedule)
        k.addrlist = copy.deepcopy(self.addrlist)
        k.error = copy.deepcopy(self.error)
        return k

    def __repr__(self):
        string = ""
        string += "Schedule Object:\n"
        string += "\tAddrlist: " + str(self.addrlist) + "\n"
        string += "\tError: " + str(self.error) + "\n"
        string += "\tNumber of Threads: " + str(self.getNumberOfThreads())
        string += "\n"
        string += "\tLength of Schedule: " + str(self.getScheduleLength()) 
        string += "\n"
        string += "\tCtxt Switches: " + str(self.getContextSwitches()) + "\n"
        string += "\tPreemptions: " + str(self.getPreemptions()) + "\n"
        string += "\tSchedule:\n "
        for x in self.schedule:
            string += "\t\t" + x.__repr__() + "\n"
        string += "\n"
        return string


    def __eq__(self, other):
        if not isinstance(other, Schedule):
            return False

        if self.error != other.error:
            return False

        if self.addrlist != other.addrlist:
            return False

        if self.getScheduleLength() != other.getScheduleLength():
            return False

        i = 0

        while i < self.getScheduleLength():
            if not self.schedule[i] == other.schedule[i]:
                return False
            i += 1

        return True

    def __ne__(self, other):
        return not (self.__eq__(other))


    def getNumberOfThreads(self):
        d = {}
        for x in self.schedule:
            d[x.caller] = 1
            d[x.chosen] = 1
        return len(d.keys())


    def getContextSwitches(self):
        context_switches = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                context_switches += 1
        return context_switches


    def getPreemptions(self):
        preemptions = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                if x.caller in x.enabled:
                    preemptions += 1
        return preemptions

    def getSummaryDict(self):
        d = {}
        d["Total Blocks"] = self.getScheduleLength()
        d["Total Threads"] = self.getNumberOfThreads()
        d["Context Switches"] = self.getContextSwitches()
        d["Preemptions"] = self.getPreemptions()
        return d

    def getSummaryInfo(self):
        string = "Total Blocks: " + str(self.getScheduleLength()) + "\n"
        string += "Total Threads: " + str(self.getNumberOfThreads()) + "\n"
        string += "Context Switches: " + str(self.getContextSwitches()) + "\n"
        string += "Preemptions: " + str(self.getPreemptions()) + "\n"
        return string

    def parseFile(self, file_in):
        assert os.path.exists(file_in)
        schedin = open(file_in, "r").readlines()
        assert "begin_addr_list" in schedin[0]
        schedin.pop(0)
        while True:
            item = schedin.pop(0)
            if "end_addr_list" in item:
                break
            self.addrlist.append(item.strip())

        while True:
            if len(schedin) < 1:
                self.error = "segfault " + self.schedule[-1].chosen
                break
            item = schedin.pop(0).strip()
            if "SCHED" in item:
                assert len(schedin) >= 5
                tmp_list = schedin[:5]
                tmp_point = SchedulePoint(tmp_list)
                self.schedule.append(tmp_point)
                schedin = schedin[5:]
                if len(schedin) > 0 and "assert" not in schedin[0]:
                    assert tmp_point.chosen in tmp_point.enabled
            elif "SIGNAL" in item:
                assert len(schedin) >= 11
                tmp_list = schedin[:11]
                tmp_point = SchedulePoint(tmp_list)
                self.schedule.append(tmp_point)
                schedin = schedin[11:]
                if len(schedin) > 0 and "assert" not in schedin[0]:
                    assert tmp_point.chosen in tmp_point.enabled
                    assert tmp_point.signalled in tmp_point.oncond
            elif "assert" in item:
                self.error = item.strip()
                break
            elif "end_log" in item:
                self.error = None
                break
            else:
                print "Unknown item:", item
                assert False

    def outputScheduleStep(self, file_out, sched, times, addr, skip, sig):
        assert not file_out.closed
        file_out.write(sched + ":" + str(times) + "\n")
        file_out.write(str(addr) + ":" + str(skip) + "\n")
        for x in sig:
            file_out.write(x + " ")
        file_out.write("##\n")

    def outputRelaxedSchedule(self, file_out):
        assert len(self.schedule) > 0
        fout = open(file_out, "w")
        fout.write("begin_addr_list\n")
        for x in self.addrlist:
            fout.write(x + "\n")
        fout.write("end_addr_list\n")
        times_scheduled = 0
        addr_dict = {}
        signal_list = []
        i = 0
        lastSched = None

        while i < len(self.schedule):
            tmp = self.schedule[i]
            if lastSched is None:
                assert tmp.caller == "0"
                lastSched = tmp
                times_scheduled = 1
                addr_dict[tmp.addr] = 1
                if self.isSignal(tmp):
                    signal_list.append(tmp.signalled)
                i += 1
                continue
            if tmp.chosen != lastSched.chosen:
                if not addr_dict.has_key(tmp.addr):
                    addr_dict[tmp.addr] = 0
                self.outputScheduleStep(fout, lastSched.chosen, \
                        times_scheduled, tmp.addr, addr_dict[tmp.addr],  \
                        signal_list)
                addr_dict = {}
                times_scheduled = 0
                signal_list = []
            if not addr_dict.has_key(tmp.addr):
                addr_dict[tmp.addr] = 0
            addr_dict[tmp.addr] += 1
            times_scheduled += 1
            if self.isSignal(tmp):
                signal_list.append(tmp.signalled)

            lastSched = tmp
            i += 1

        self.outputScheduleStep(fout, lastSched.chosen, \
                 times_scheduled, "0", "0", signal_list) 

    def outputStrictSchedule(self, file_out):
        assert len(self.schedule) > 0
        assert False
        pass

    def getThreadOrder(self):
        assert len(self.schedule) > 0
        thread_order = []
        thread_order.append(self.schedule[0].caller)
        for x in self.schedule:
            if x.chosen != x.caller:
                thread_order.append(x.chosen)

        return thread_order

    def isSignal(self, sched):
        return hasattr(sched, "signalled")

    #returns a Schedule object of the new schedule
    #if block can be removed, otherwise None (if thread cannot
    #be found or a valid schedule cannot be generated)
    def removeFinalBlockOfThread(self, thread):
        assert len(self.schedule) > 0
        j = 0
        i = len(self.schedule) - 1
        while i >= 0:
            if self.schedule[i].chosen == thread:
                j = i
                while j >= 0:
                    if self.schedule[j].chosen != thread:
                        break
                    j -= 1
                break
            i -= 1

        if i < j:
            return None
        if j == i:
            return None

        assert j < i

        copy_schedule = copy.deepcopy(self.schedule)

        copy_front = copy_schedule[:j+1]
        copy_back = copy_schedule[i+1:]

        final_list = copy_front + copy_back
        self.patchUpSchedule(final_list)

        final_sched = Schedule()
        final_sched.schedule = final_list
        final_sched.error = self.error
        final_sched.addrlist = self.addrlist
        return final_sched


    def patchUpSchedule(self, sched):
        last = "0"
        for x in sched:
            x.caller = last
            last = x.chosen
            
    def consolidateFrontierForward(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        if frontier + 1 >= len(self.schedule):
            return None
        
        target = self.schedule[frontier].chosen


        if target == self.schedule[frontier + 1].chosen:
            return None
        if target not in self.schedule[frontier + 1].enabled:
            return None 
        
        block_start = frontier + 1
        block_end = 0

        while block_start < len(self.schedule):
            tmp_chosen = self.schedule[block_start].chosen
            if tmp_chosen == target:
                block_end = block_start
                while block_end < len(self.schedule):
                    tmp_end = self.schedule[block_end].chosen
                    if tmp_end != target:
                        break
                    block_end += 1
                break
            block_start += 1

        if block_start == len(self.schedule):
            return None

        copy_schedule = copy.deepcopy(self.schedule)

        
        prefrontier = copy_schedule[:frontier + 1]
        postfrontier = copy_schedule[frontier + 1: block_start]
        mobileblock = copy_schedule[block_start:block_end]
        end = copy_schedule[block_end:]

        #address swaps for the benenfit of relaxed scheduling
        addrsave = mobileblock[0].addr
        mobileblock[0].addr = postfrontier[0].addr
        if len(end) > 0:
            postfrontier[0].addr = end[0].addr
            end[0].addr = addrsave
        else:
            postfrontier[0].addr = "0"

        final_list = prefrontier + mobileblock + postfrontier + end
        self.patchUpSchedule(final_list)

        assert len(final_list) == len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = final_list
        final_sched.error = None
        final_sched.addrlist = self.addrlist

        return final_sched


    def consolidateFrontierBackward(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        if frontier == 0:
            return None

        target = self.schedule[frontier].chosen

        if target == self.schedule[frontier - 1].chosen:
            return None

        block_end = frontier - 1
        block_start = frontier

        while block_end >= 0:
            tmp_chosen = self.schedule[block_end].chosen
            if tmp_chosen == target:
                block_start = block_end
                while block_start >= 0:
                    tmp_start = self.schedule[block_start].chosen
                    if tmp_start != target:
                        break
                    block_start -= 1
                break
            block_end -= 1

        if block_end == -1:
            return None

        start = []
        mobileblock = []
        prefrontier = []
        postfrontier = []

        copy_schedule = copy.deepcopy(self.schedule)


        if block_start == -1:
            mobileblock = copy_schedule[:block_end + 1]
            prefrontier = copy_schedule[block_end + 1: frontier]
            postfrontier = copy_schedule[frontier:]

        else:
            start = copy_schedule[:block_start + 1]
            mobileblock = copy_schedule[block_start + 1: block_end + 1]
            prefrontier = copy_schedule[block_end + 1:frontier]
            postfrontier = copy_schedule[frontier:]

        #address reassignment
        addrsave = prefrontier[0].addr
        prefrontier[0].addr = mobileblock[0].addr
        mobileblock[0].addr = postfrontier[0].addr
        postfrontier[0].addr = addrsave

        final_list = start + prefrontier + mobileblock + postfrontier
        self.patchUpSchedule(final_list)
        assert len(final_list) == len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = final_list
        final_sched.error = None
        final_sched.addrlist = self.addrlist 
        return final_sched


    def getScheduleLength(self):
        return len(self.schedule)
        



def testSchedulePointConstructor():
    tmp = ["chosen:1", "caller:0", "typstr:Before_Mutex_lock"] 
    tmp.append("idaddr:0x100")
    tmp.append("enabled:1,2,3,")
    s = SchedulePoint(tmp)
    assert s.chosen == "1"
    assert s.caller == "0"
    assert s.type == "Before_Mutex_lock"
    assert s.addr == "0x100"
    assert s.enabled == ["1", "2", "3"]


    
    tmp[1] = "callee:0"
    try:
        s = SchedulePoint(tmp)
    except:
        pass
    else:
        assert False


    tmp[0] = "chosen:1"
    tmp[1] = "caller:0"
    tmp[2] = "typstr:Simulate_Signal"
    newtmp = ["signalled:3", "caller:0", "idaddr:0x100"]
    newtmp.append("cond:0x100") 
    newtmp.append("oncond:3,4")
    newtmp.append("SCHED")

    tmp = newtmp + tmp

    s = SchedulePoint(tmp)
    assert s.signalled == "3"
    assert s.cond == "0x100"
    assert s.oncond == ["3", "4"]


    try:
        s = SchedulePoint([])
    except:
        pass
    else:
        assert False
    
    return 1

def testSchedulePointEquality():
    tmp = ["chosen:1", "caller:0", "typstr:Before_Mutex_lock"] 
    tmp.append("idaddr:0x100")
    tmp.append("enabled:1,2,3,")
    
    tmp2 = ["signalled:3", "caller:0", "idaddr:0x100"]
    tmp2.append("cond:0x100") 
    tmp2.append("oncond:3,4")
    tmp2.append("SCHED")
    tmp2 = tmp2 + ["chosen:1", "caller:0", "typstr:Simulate_Signal"] 
    tmp2.append("idaddr:0x100")
    tmp2.append("enabled:1,2,3,")

    s1 = SchedulePoint(tmp)
    s2 = SchedulePoint(tmp)
    assert s1 == s2
    s2.enabled = ["1", "2"]
    assert s1 != s2
    s2 = SchedulePoint(tmp2)
    assert s1 != s2
    s1 = SchedulePoint(tmp2)
    assert s1 == s2
    #cond addr can be dynamic, so don't check for equality here 
    s2.cond = "0x100"
    assert s1 == s2
    s2 = SchedulePoint(tmp2)
    s2.signalled = "4"
    assert s1 != s2
    return 1

def testScheduleEquality():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s1 = Schedule(test5)
    s2 = Schedule(test5)
    assert s1 == s2
    s1.error = "segfault"
    assert s1 != s2
    s1 = Schedule(test5)
    assert s1 == s2
    s1.addrlist = ["all"]
    assert s1 != s2
    s1 = Schedule(test5)
    assert s1 == s2
    s1.schedule[3].caller = ["1"]
    assert s1 != s2
    return 1



def testParse():
    try:
        s = Schedule("/NOTADIR/")
    except:
        pass
    else:
        assert False

    s = Schedule()
    assert len(s.schedule) == 0
    assert len(s.addrlist) == 0
    assert s.error is None

    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)

    test1 = os.path.join(td, "01.sched")
    assert os.path.exists(test1)
    s = Schedule(test1)
    assert len(s.schedule) == 2
    assert s.error is None
    assert s.addrlist == []
    assert s.schedule[1].cond == "0x501380"
    assert s.schedule[0].enabled == ["1"]
    assert s.schedule[1].enabled == ["1", "0"]
    assert not hasattr(s.schedule[0], "cond")
    

    test2 = os.path.join(td, "02.sched")
    assert os.path.exists(test2)
    s = Schedule(test2)
    assert len(s.schedule) == 2
    assert s.error == "segfault 0" 
    assert s.addrlist == ["all"]
    assert s.schedule[1].caller == "1"
    assert s.schedule[0].type == "Before_Mutex_Lock"


    
    test3 = os.path.join(td, "03.sched")
    assert os.path.exists(test3)
    s = Schedule(test3)
    assert s.error == "assert fail ../serializer/executiontracker.cpp 163"

    # 0 is not oncond
    test10 = os.path.join(td, "10.sched")
    assert os.path.exists(test10)
    try:
        s = Schedule(test10)
    except:
        pass
    else:
        assert False

    # 1 is not enabled at first sched point 
    test11 = os.path.join(td, "11.sched")
    assert os.path.exists(test11)
    try:
        s = Schedule(test11)
    except:
        pass
    else:
        assert False

    #caller that schedules at signal is not the same as the
    #caller of simulate signal
    test7 = os.path.join(td, "07.sched")
    assert os.path.exists(test7)
    try:
        s = Schedule(test7)
    except:
        pass
    else:
        assert False


    return 1

# Fixed point check
#  -generate a schedule (libserializer)
#  -read it in, and output it in relaxed form (sched1)
#  -replay execution from the relaxed ouput
#  -read in the generated schedule from last execution
#  -make sure a relaxed sched generated from that equals sched1
def testOutputRelaxedSched(target):
    cwd = os.getcwd()
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    os.chdir(td)
    fout = open("logfile", "w")
    assert os.path.exists(td)
    libser = os.path.join(tr, "bin", "libserializer.so")
    assert os.path.exists(libser)
    librelaxser = os.path.join(tr, "bin", "librelaxedserial.so")
    assert os.path.exists(librelaxser)
    assert os.path.exists(target)

    args = ["g++", "-lpthread", "-g", target]

    exit = subprocess.call(args)
    assert exit == 0
    assert os.path.exists("a.out")

    #get initial schedule
    os.environ["LD_PRELOAD"] = libser
    args = ["./a.out"]
    exit = subprocess.call(args, stdout=fout, stderr=fout)
    assert exit == 0
    assert os.path.exists("my-schedule")
    del os.environ["LD_PRELOAD"]

    #read in and process schedule
    s = Schedule("my-schedule")
    s.outputRelaxedSchedule("thrille-relaxed-sched")
    assert os.path.exists("thrille-relaxed-sched")
    os.remove("my-schedule")

    #execute relaxed schedule
    os.environ["LD_PRELOAD"] = librelaxser
    args = ["./a.out"]
    exit = subprocess.call(args, stdout=fout, stderr=fout)
    assert exit == 0
    assert os.path.exists("my-schedule")
    del os.environ["LD_PRELOAD"]

    #new relaxed schedule
    s = Schedule("my-schedule")
    s.outputRelaxedSchedule("sched2")
    assert os.path.exists("sched2")

    #make sure we reached a fixed point
    in1 = open("thrille-relaxed-sched", "r").readlines()
    in2 = open("sched2", "r").readlines()

    assert in1 == in2
    fout.close()
    os.remove("logfile")
    os.remove("my-schedule")
    os.remove("thrille-relaxed-sched")
    os.remove("sched2")
    os.remove("a.out")
    os.chdir(cwd)
    return 1

def testOutputRelaxedSched2(target):
    cwd = os.getcwd()
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    os.chdir(td)
    fout = open("logfile", "w")
    assert os.path.exists(td)
    libser = os.path.join(tr, "bin", "libserializer.so")
    assert os.path.exists(libser)
    librelaxser = os.path.join(tr, "bin", "librelaxedserial.so")
    assert os.path.exists(librelaxser)
    assert os.path.exists(target)

    args = ["g++", "-lpthread", "-g", target]

    exit = subprocess.call(args)
    assert exit == 0
    assert os.path.exists("a.out")

    #execute relaxed schedule
    shutil.copy("passpoint.sched", "thrille-relaxed-sched")
    os.environ["LD_PRELOAD"] = librelaxser
    args = ["./a.out"]
    exit = subprocess.call(args, stdout=fout, stderr=fout)
    assert exit == 0
    assert os.path.exists("my-schedule")
    del os.environ["LD_PRELOAD"]

    #new relaxed schedule
    s = Schedule("my-schedule")
    s.outputRelaxedSchedule("sched2")
    assert os.path.exists("sched2")

    #make sure we reached a fixed point
    in1 = open("thrille-relaxed-sched", "r").readlines()
    in2 = open("sched2", "r").readlines()

    sched60 = in1.pop(2)
    sched48 = in2.pop(2)
    assert sched60 == "0:60\n"
    assert sched48 == "0:48\n"

    assert in1 == in2

    #try fixed point trick one more time
    shutil.copy("sched2", "thrille-relaxed-sched")
    os.environ["LD_PRELOAD"] = librelaxser
    args = ["./a.out"]
    exit = subprocess.call(args, stdout=fout, stderr=fout)
    assert exit == 0
    assert os.path.exists("my-schedule")
    del os.environ["LD_PRELOAD"]

    s = Schedule("my-schedule")
    s.outputRelaxedSchedule("sched3")
    assert os.path.exists("sched3")

    in1 = open("sched3").readlines()
    in2 = open("sched2").readlines()

    assert in1 == in2

    fout.close()
    os.remove("logfile")
    os.remove("my-schedule")
    os.remove("thrille-relaxed-sched")
    os.remove("sched2")
    os.remove("sched3")
    os.remove("a.out")
    os.chdir(cwd)
    return 1

def testGetThreadOrder():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)

    test4 = os.path.join(td, "04.sched")
    assert os.path.exists(test4)
    s = Schedule(test4)
    tmp = s.getThreadOrder()
    assert tmp == ["0", "1", "0"]
    
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.getThreadOrder()
    assert tmp == ["0", "2", "0", "1", "0"]

    return 1

def testRemoveFinalBlock():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("0")
    assert tmp is not None
    assert len(tmp.schedule) == 6
    assert tmp.schedule[0].chosen == "2"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "0"
    assert tmp.schedule[3].chosen == "1"
    assert tmp.schedule[4].chosen == "1"
    assert tmp.schedule[5].chosen == "1"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "2", "0", "1"]

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("1")
    assert tmp is not None
    assert len(tmp.schedule) == 4
    assert tmp.schedule[0].chosen == "2"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "0"
    assert tmp.schedule[3].chosen == "0"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "2",  "0"]
    assert tmp.schedule[-1].addr == "0x400b25"

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("2")
    assert tmp is not None
    assert len(tmp.schedule) == 6
    assert tmp.schedule[0].chosen == "0"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "1"
    assert tmp.schedule[3].chosen == "1"
    assert tmp.schedule[4].chosen == "1"
    assert tmp.schedule[5].chosen == "0"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "1", "0"]

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("3")
    assert tmp is None
    return 1



def testIsSignal():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)

    assert not s.isSignal(s.schedule[4])
    assert s.isSignal(s.schedule[5])
    return 1


def testConsolidateForward():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(0)
    assert newsched is None


    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(1)
    assert newsched is None

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(2)
    assert newsched is not None
    assert newsched.getThreadOrder() == ["0", "2", "0", "1"]
    assert len(newsched.schedule) == 7
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "0"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "1"

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(3)
    assert newsched is None

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(4)
    assert newsched is None

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(5)
    assert newsched is None

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(6)
    assert newsched is None

    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    newsched = s.consolidateFrontierForward(4)
    assert newsched is not None
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "3", "1", "0"]
    assert len(newsched.schedule) == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "3"
    assert newsched.schedule[6].chosen == "1"
    assert newsched.schedule[7].chosen == "0"
    return 1

def testConsolidateReverse():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    length = s.getScheduleLength() - 1
    assert length == 7
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is not None
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "1", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "1"
    assert newsched.schedule[2].chosen == "3"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "0"
    assert newsched.schedule[6].chosen == "0"
    assert newsched.schedule[7].chosen == "0"

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is not None
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "3"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"
    
    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is not None
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "3"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is None

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is None

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is None

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is None

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is None

    try:
        length -= 1
        newsched = s.consolidateFrontierBackward(length)
    except:
        pass
    else:
        assert False

    return 1

#list slicing does shallow copies--oops
def testFailureOne():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    length = s.getScheduleLength() - 1
    assert length == 7
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is not None
    assert s.schedule[0].caller != "5"
    newsched.schedule[0].caller = "5"
    assert newsched.schedule[0].caller == "5"
    assert s.schedule[0].caller != "5"
    return 1

def testFailureTwo():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(2)
    assert newsched is not None
    assert s.schedule[0].caller != "banana"
    newsched.schedule[0].caller = "banana"
    assert newsched.schedule[0].caller == "banana"
    assert s.schedule[0].caller != "banana"
    return 1

def testFailureThree():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("1")
    assert tmp is not None
    assert s.schedule[0].addr != "shallow_copy_fail"
    tmp.schedule[0].addr = "shallow_copy_fail"
    assert tmp.schedule[0].addr == "shallow_copy_fail"
    assert s.schedule[0].addr != "shallow_copy_fail"
    return 1

# have to do manipulations of the id's were we switch 
# when we manipulate schedule
def testFailureFour():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test = os.path.join(td, "13.sched")
    assert os.path.exists(test)
    s = Schedule(test)
    newsched = s.consolidateFrontierForward(2)
    assert newsched is not None
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "4"]
    assert len(newsched.schedule) == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "0"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "1"
    assert newsched.schedule[7].chosen == "4"
    assert newsched.schedule[3].addr == s.schedule[3].addr
    assert newsched.schedule[4].addr == s.schedule[7].addr
    assert newsched.schedule[7].addr == s.schedule[6].addr
    return 1

def testFailureFive():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    length = s.getScheduleLength() - 1
    assert length == 7
    newsched = s.consolidateFrontierBackward(length)
    assert newsched is not None
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "1", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "1"
    assert newsched.schedule[2].chosen == "3"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "0"
    assert newsched.schedule[6].chosen == "0"
    assert newsched.schedule[7].chosen == "0"
    assert newsched.schedule[1].addr == s.schedule[1].addr
    assert newsched.schedule[5].addr == s.schedule[7].addr
    assert newsched.schedule[7].addr == s.schedule[3].addr

    return 1

def testGetNumberOfThreads():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getNumberOfThreads() == 3
    s.schedule[0].chosen = "0"
    assert s.getNumberOfThreads() == 3
    s.schedule[1].caller = "0"
    assert s.getNumberOfThreads() == 2
    s.schedule[5].caller = "10"
    assert s.getNumberOfThreads() == 3
    return 1

def testGetContextSwitches():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getContextSwitches() == 4
    s.schedule[0].chosen = "0"
    assert s.getContextSwitches() == 3
    del s.schedule[6]
    assert s.getContextSwitches() == 2
    return 1

def testGetPreemptions():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getPreemptions() == 3
    s.schedule[1].enabled = ["2"]
    assert s.getPreemptions() == 4
    return 1

def testGetScheduleLength():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getScheduleLength() == 7
    del s.schedule[3]
    assert s.getScheduleLength() == 6
    return 1


def test():
    test_passed = 0
    test_passed += testSchedulePointConstructor()
    test_passed += testSchedulePointEquality()
    test_passed += testScheduleEquality()
    test_passed += testGetNumberOfThreads()
    test_passed += testGetContextSwitches()
    test_passed += testGetPreemptions()
    test_passed += testGetScheduleLength()
    test_passed += testParse()
    test_passed += testOutputRelaxedSched("08danger.cpp")
    test_passed += testOutputRelaxedSched2("passpoint.cpp")
    test_passed += testGetThreadOrder()
    test_passed += testRemoveFinalBlock()
    test_passed += testIsSignal()
    test_passed += testConsolidateForward()
    test_passed += testConsolidateReverse()
    test_passed += testFailureOne()
    test_passed += testFailureTwo()
    test_passed += testFailureThree()
    test_passed += testFailureFour()
    test_passed += testFailureFive()

    print "Tests passed:", test_passed



if __name__ == "__main__":
    test()

