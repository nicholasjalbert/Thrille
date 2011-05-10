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

# represents one scheduling decision
# at a synchronization call/memory access
class SchedulePoint:

    def __init__(self, list):
        assert len(list) >= 0
        if "signalled" in list[0]:
            assert len(list) == 13
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
            assert "memor1" in list[10]
            assert "memor2" in list[11]
            assert "enable" in list[12]
            self.chosen = self.splitColonSeparatedInput(list[6])
            self.caller = self.splitColonSeparatedInput(list[7])
            self.type = self.splitColonSeparatedInput(list[8])
            self.addr = self.splitColonSeparatedInput(list[9])
            self.memory_1 = self.splitColonSeparatedInput(list[10])
            self.memory_2 = self.splitColonSeparatedInput(list[11])
            self.enabled = self.splitCommaSeparatedInput(list[12])
            self.signalled = self.splitColonSeparatedInput(list[0])
            sanitycheck = self.splitColonSeparatedInput(list[1])
            self.cond = self.splitColonSeparatedInput(list[3])
            self.oncond = self.splitCommaSeparatedInput(list[4])
            assert sanitycheck == self.caller
        else:
            assert len(list) == 7
            assert "chosen" in list[0]
            assert "caller" in list[1]
            assert "typstr" in list[2]
            assert "idaddr" in list[3]
            assert "memor1" in list[4]
            assert "memor2" in list[5]
            assert "enable" in list[6]
            self.chosen = self.splitColonSeparatedInput(list[0])
            self.caller = self.splitColonSeparatedInput(list[1])
            self.type = self.splitColonSeparatedInput(list[2])
            self.addr = self.splitColonSeparatedInput(list[3])
            self.memory_1 = self.splitColonSeparatedInput(list[4])
            self.memory_2 = self.splitColonSeparatedInput(list[5])
            self.enabled = self.splitCommaSeparatedInput(list[6])

    def __repr__(self):
        string = "SchedulePoint(" + "chosen: " + str(self.chosen) + ", "
        string += "caller: " + self.caller + ", "
        string += "type: " + self.type + ", "
        string += "addr: " + self.addr + ", "
        string += "memor1: " + self.memory_1 + ", "
        string += "memor2: " + self.memory_2 + ", "
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
        #if self.addr != other.addr:
            #return False
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

    def isSignal(self):
        return hasattr(self, "signalled")

    def isCheckpoint(self):
        return "Thrille_Checkpoint" in self.type

class ScheduleSegment(object):
    def __init__(self, _start, _end, _tid, _count, _read, _written, _join, _create, _bar, _en):
        self.start = _start
        self.end = _end
        self.tid = _tid
        self.count = _count
        self.read = _read
        self.written = _written
        self.joined = _join
        self.created = _create
        self.barrier = _bar
        self.enabled = _en

    def __repr__(self):
        rep = "Segment("
        rep += "start:" + str(self.start) + ", "
        rep += "end:" + str(self.end) + ", "
        rep += "thread:" + str(self.tid) + ", "
        rep += "count:" + str(self.count) + ", "
        rep += "read:" + str(self.read) + ", "
        rep += "written:" + str(self.written) + ", "
        rep += "joined:" + str(self.joined) + ", "
        rep += "created:" + str(self.created) + ", "
        rep += "barrier:" + str(self.barrier) + ", "
        rep += "enabled:" + str(self.enabled) + ")"
        return rep

    def repOK(segmented_schedule):
        progress = {}
        for x in segmented_schedule:
            if x.tid in progress:
                assert progress[x.tid] == x.start
            progress[x.tid] = x.end
            en_len = len(x.enabled)
            if en_len == 1 and x.enabled[0] == "PYTHON_GEN":
                pass
            else:
                assert x.tid in x.enabled
    
    repOK = staticmethod(repOK)

    #remove mention of threads that are never scheduled
    #TODO: might be a mess if thread 7 out of 10 is never scheduled
    def sanitize(segmented_schedule):
        threads = set()
        for segment in segmented_schedule:
            threads.add(int(segment.tid))


        for segment in segmented_schedule:
            if segment.created is not None:
                if int(segment.created) not in threads:
                    segment.created = None
            if segment.joined is not None:
                if int(segment.joined) not in threads:
                    segment.joined = None
            i = 0
            while i < len(segment.enabled):
                thr = int(segment.enabled[i])
                if thr not in threads:
                    del segment.enabled[i]
                else:
                    i += 1
            if segment.barrier is not None:
                while i < len(segment.barrier):
                    thr, ev = segment.barrier[i]
                    thr = int(thr)
                    if thr not in threads:
                        del segment.barrier[i]
                    else:
                        i += 1
    
    sanitize = staticmethod(sanitize)

# represents the sum total schedule of a program execution
# composed of a list of schedule blocks

class Schedule(object):

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
        i = 0
        for x in self.schedule:
            string += "\t\t(" + str(i) + ") " + x.__repr__() + "\n"
            i += 1
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


    def getPostfixAfter(self, target_sched):
        assert self.getScheduleLength() >= target_sched.getScheduleLength()
        assert self.addrlist == target_sched.addrlist
        mutable_target_sched = copy.deepcopy(target_sched)
        mutable_me = copy.deepcopy(self)
        while mutable_target_sched.getScheduleLength() > 0:
            my_el = mutable_me.schedule.pop(0)
            other_el = mutable_target_sched.schedule.pop(0)
            if hasattr(my_el, "signalled"):
                assert my_el.signalled == other_el.signalled
                if other_el.chosen is None:
                    assert mutable_target_sched.getScheduleLength() == 0
                    my_el.signalled = None
                    mutable_me.schedule.insert(0, my_el)
                else:
                    assert my_el.chosen == other_el.chosen

            else:
                assert my_el.chosen == other_el.chosen
        return mutable_me

    def getNumberOfThreads(self):
        d = {}
        for x in self.schedule:
            d[x.caller] = 1
            d[x.chosen] = 1
        return len(d.keys())

    def getError(self):
        return self.error

    def _getContextSwitches(self):
        context_switches = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                context_switches += 1
        return context_switches
    
    def _getNonPreemptiveContextSwitches(self):
        npcs = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                if not (x.caller in x.enabled):
                    npcs += 1
        return npcs 


    def _getPreemptions(self):
        preemptions = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                if x.caller in x.enabled:
                    preemptions += 1
        return preemptions

    def getContextSwitches(self):
        context_switches = self._getContextSwitches()
        npcs = self._getNonPreemptiveContextSwitches()
        preemptions = self._getPreemptions()
        assert (npcs + preemptions) == context_switches
        return context_switches
    
    def getNonPreemptiveContextSwitches(self):
        context_switches = self._getContextSwitches()
        npcs = self._getNonPreemptiveContextSwitches()
        preemptions = self._getPreemptions()
        assert (npcs + preemptions) == context_switches
        return npcs

    def getPreemptions(self):
        context_switches = self._getContextSwitches()
        npcs = self._getNonPreemptiveContextSwitches()
        preemptions = self._getPreemptions()
        assert (npcs + preemptions) == context_switches
        return preemptions

    def isInLastTEI(self, i):
        if i >= len(self.schedule):
            # can happen with signals
            return True
        assert i >= 0

        pos = i
        thr = self.schedule[i].chosen
        while pos < len(self.schedule):
            if self.schedule[pos].chosen != thr:
                return False
            pos += 1

        return True

    def getNumberOfCheckpoints(self):
        checkpoints = 0
        for x in self.schedule:
            if x.isCheckpoint():
                checkpoints += 1
        return checkpoints



    def getSignalSchedule(self):
        tid_segment_count = {}
        signal_dict = {}
        if len(self.schedule) == 0:
            return signal_dict

        for event in self.schedule:
            if event.caller in tid_segment_count:
                tid_segment_count[event.caller] += 1
            else:
                #TODO duplicate code
                tid_segment_count[event.caller] = 0
            if event.isSignal():
                if event.caller not in signal_dict:
                    signal_dict[event.caller] = {}
                eid = tid_segment_count[event.caller]
                signal_dict[event.caller][eid] = event.signalled
        return signal_dict

    def segmentSchedule(self):
        segmented_schedule = []
        tid_segment_count = {}
        next_up_read = {}
        next_up_write = {}
        next_up_startid = {}
        if len(self.schedule) == 0:
            return []

        prev_en = ["1"]
        join_dict = {}

        for event in self.schedule:
            if event.caller not in tid_segment_count:
                tid_segment_count[event.caller] = 0

            if event.caller not in next_up_startid:
                next_up_startid[event.caller] = "0x0"

            if event.caller not in next_up_read:
                next_up_read[event.caller] = set()
             
            if event.caller not in next_up_write:
                next_up_write[event.caller] = set()
           
            created = None
            if "After_Create" in event.type:
                created = set(event.enabled) - set(prev_en) 
                assert len(created) == 1
                created = list(created)[0]

            barrier = None
            if "Before_Barrier_Wait" in event.type:
                barrier_id = event.memory_1
                assert barrier_id != "0x0"
                next_up_write[event.caller].add(barrier_id)

            joined = None
            if event.caller in join_dict:
                joined = join_dict[event.caller]
                join_str = hex(int(joined))
                assert join_str in next_up_write[event.caller]
                next_up_write[event.caller] -= set([join_str])
                del join_dict[event.caller]

            segmented_schedule.append(ScheduleSegment( \
                    next_up_startid[event.caller], event.addr, \
                    event.caller, tid_segment_count[event.caller], \
                    next_up_read[event.caller], next_up_write[event.caller], \
                    joined, created, barrier, prev_en))
            

            prev_en = event.enabled[:]
            tid_segment_count[event.caller] += 1
            next_up_startid[event.caller] = event.addr

            if "Join" in event.type:
                assert "0x" in event.memory_1[:2]
                assert event.memory_2 == "0x0"
                join_dict[event.caller] = str(int(event.memory_1,0))

            if event.memory_1 != "0x0" and "Read" not in event.type:
                next_up_read[event.caller] = set()
                next_up_write[event.caller] = set([event.memory_1])
                if event.memory_2 != "0x0":
                    next_up_write[event.caller].add(event.memory_2)
            elif event.memory_1 != "0x0" and "Read" in event.type:
                assert event.memory_2 == "0x0"
                next_up_read[event.caller] = set([event.memory_1])
                next_up_write[event.caller] = set()
            else:
                next_up_read[event.caller] = set()
                next_up_write[event.caller] = set()

        last = self.schedule[-1]
        if last.chosen not in tid_segment_count:
            tid_segment_count[last.chosen] = 0
        if last.chosen not in next_up_startid:
            next_up_startid[last.chosen] = "0x1"
        if last.chosen not in next_up_read:
            next_up_read[last.chosen] = set()
        if last.chosen not in next_up_write:
            next_up_write[last.chosen] = set()
 
        joined = None
        if last.chosen in join_dict:
            joined = join_dict[last.chosen]
            join_str = hex(int(joined))
            assert join_str in next_up_write[last.chosen]
            next_up_write[last.chosen] -= set([join_str])
            del join_dict[last.chosen]
        
        barrier = None

        segmented_schedule.append(ScheduleSegment( \
                next_up_startid[last.chosen], "0x2", \
                last.chosen, tid_segment_count[last.chosen], \
                next_up_read[last.chosen], next_up_write[last.chosen], \
                joined, None, barrier, last.enabled))
        ScheduleSegment.sanitize(segmented_schedule)
        ScheduleSegment.repOK(segmented_schedule)
        #for x in segmented_schedule:
        #    print x
        #sys.exit(0)
        return segmented_schedule
    
    def countThreadEvents(self, threadid):
        segmented_schedule = self.segmentSchedule()
        segmented_schedule.reverse()
        for x in segmented_schedule:
            if int(x.tid) == int(threadid):
                return x.count + 1
        assert False, "thread %s not found!" % str(threadid)


    # -returns a list of tuples:
    #     (first_thread_id, first_event_id, second_thread_id, second_event_id)
    # -thread_idx determines whether thread_ids start at 0 or 1
    # -event_idx determines whether per-thread event_ids start at 0 or 1
    def getHBConstraints(self, thread_idx, event_idx):
        thread_idx = int(thread_idx)
        assert thread_idx == 0 or thread_idx == 1
        t_mod = 0
        if thread_idx == 0:
            t_mod = 1

        event_idx = int(event_idx)
        assert event_idx == 0 or event_idx == 1
        e_mod = event_idx
        
        segmented_schedule = self.segmentSchedule()
        
        # Memory conflict HB
        constraints = []
        for i in range(len(segmented_schedule)):
            event = segmented_schedule[i]
            for x in event.read:
                j = i - 1
                while j >= 0:
                    tmp = segmented_schedule[j]
                    if x in tmp.written:
                        ft = int(tmp.tid) - t_mod
                        fe = int(tmp.count) + e_mod
                        st = int(event.tid) - t_mod
                        se = int(event.count) + e_mod
                        constraints.append((ft, fe, st, se))
                        break
                    j -= 1
            for x in event.written:
                j = i - 1
                while j >= 0:
                    tmp = segmented_schedule[j]
                    if x in tmp.written or x in tmp.read:
                        ft = int(tmp.tid) - t_mod
                        fe = int(tmp.count) + e_mod
                        st = int(event.tid) - t_mod
                        se = int(event.count) + e_mod
                        constraints.append((ft, fe, st, se))
                        break
                    j -= 1

        # Create HB
        for x in segmented_schedule:
            if x.created != None:
                ft = int(x.tid) - t_mod
                fe = int(x.count) + e_mod
                st = int(x.created) - t_mod
                se = 0 + e_mod
                constraints.append((ft, fe, st, se))

        # Join HB
        for i in range(len(segmented_schedule)):
            x = segmented_schedule[i]
            if x.joined != None:
                ft = int(x.joined) - t_mod
                fe = None
                j = i - 1
                while j >= 0:
                    if int(segmented_schedule[j].tid) == int(x.joined):
                        fe = int(segmented_schedule[j].count) + e_mod
                        break;
                    j -= 1
                else:
                    assert False, "joining thread not found"
                st = int(x.tid) - t_mod
                se = int(x.count) + e_mod
                constraints.append((ft, fe, st, se))

        # Barrier HB
        for i in range(len(segmented_schedule)):
            x = segmented_schedule[i]
            if x.barrier != None:
                    tid, ev = x.barrier
                    ft = int(tid) - t_mod
                    fe = int(ev) + e_mod
                    st = int(x.tid) - t_mod
                    se = int(x.count) + e_mod
                    constraints.append((ft,fe,st,se))

        return constraints

    def getInitialSearchStack(self):
        segmented_schedule = self.segmentSchedule()
        stack = []
        total_events = 0
        for x in segmented_schedule:
            new_en = []
            for y in x.enabled:
                new_en.append(int(y) - 1)
            new_en.remove(int(x.tid) - 1)
            stack.append((total_events, int(x.count), int(x.tid) - 1, new_en))
            total_events += 1
        return stack

    def makeScheduleFromList(thread_list, signals, addrlist, error):
        sched = Schedule()
        sched.schedule = []
        sched.error = error
        sched.addrlist = addrlist
        if len(thread_list) == 0:
            return sched

        exec_count = {}
       
        # First event is implicit (i.e. first thread executes without being
        # "scheduled" in Thrille)
        last = str(thread_list[0])
        exec_count[thread_list[0]] = 1 

        for thread in thread_list[1:]:
            if thread not in exec_count:
                exec_count[thread] = 0
            exec_count[thread] += 1
            eid = exec_count[thread]
            tmp = []
            if thread in signals:
                if eid in signals[thread]:
                    tmp.append("signalled:%s" % str(signals[thread][eid]))
                    tmp.append("caller:%s" % str(thread))
                    tmp.append("idaddr:0x99999999")
                    tmp.append("cond:0x99999999")
                    tmp.append("oncond:PYTHON_GEN")
            tmp.append("chosen:%s" % str(thread))
            tmp.append("caller:%s" % str(last))
            last = str(thread)
            tmp.append("typstr:PYTHON_GEN")
            tmp.append("idaddr:0x99999999")
            tmp.append("memor1:0x99999999")
            tmp.append("memor2:0x99999999")
            tmp.append("enable:PYTHON_GEN")
            sched.schedule.append(SchedulePoint(tmp))
        return sched
    
    makeScheduleFromList = staticmethod(makeScheduleFromList)

    def getAvgThreadsEnabledAtCS(self):
        total_enabled = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                total_enabled += len(x.enabled)

        if self.getContextSwitches() == 0:
            return 0

        return float(total_enabled) / self.getContextSwitches()
   
    def getAvgThreadsEnabledAtPreempts(self):
        total_enabled = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                if x.caller in x.enabled:
                    total_enabled += len(x.enabled)

        if self.getPreemptions() == 0:
            return 0
        
        return float(total_enabled) / self.getPreemptions()

    def getAvgThreadsEnabledAtNons(self):
        total_enabled = 0
        for x in self.schedule:
            if x.caller != x.chosen:
                if not (x.caller in x.enabled):
                    total_enabled += len(x.enabled)
        
        if self.getNonPreemptiveContextSwitches() == 0:
            return 0

        return float(total_enabled) / self.getNonPreemptiveContextSwitches()


    def getAvgThreadsEnabledAtAll(self):
        total_enabled = 0
        for x in self.schedule:
            total_enabled += len(x.enabled)

        if self.getScheduleLength() == 0:
            return 0

        return float(total_enabled) / self.getScheduleLength()


    def getSummaryDict(self):
        d = {}
        d["Total Schedule Points"] = self.getScheduleLength()
        d["Total Threads"] = self.getNumberOfThreads()
        d["Context Switches"] = self.getContextSwitches()
        d["Non-Preemptive Context Switches"] = \
                self.getNonPreemptiveContextSwitches()
        d["Preemptions"] = self.getPreemptions()
        return d

    def getSummaryInfo(self):
        string = "Total Schedule Points: " + str(self.getScheduleLength()) 
        string += "\n"
        string += "Total Threads: " + str(self.getNumberOfThreads()) + "\n"
        string += "Context Switches: " + str(self.getContextSwitches()) + "\n"
        string += "Non-Preemptive Context Switches: "
        string += str(self.getNonPreemptiveContextSwitches()) + "\n"
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
                self.error = "unknown error by " + self.schedule[-1].chosen
                break
            item = schedin.pop(0).strip()
            if "SCHED" in item:
                assert len(schedin) >= 7
                tmp_list = schedin[:7]
                tmp_point = SchedulePoint(tmp_list)
                self.schedule.append(tmp_point)
                schedin = schedin[7:]
                if len(schedin) > 0 and "assert" not in schedin[0]:
                    en_size = len(tmp_point.enabled)
                    if en_size == 1 and "PYTHON_GEN" == tmp_point.enabled[0]:
                        pass
                    else:
                        assert tmp_point.chosen in tmp_point.enabled
            elif "SIGNAL" in item:
                if len(schedin) < 13:
                    assert len(schedin) > 5
                    if "assert" in schedin[5]:
                        self.error = schedin[5].strip()
                        break
                    assert False
                tmp_list = schedin[:13]
                tmp_point = SchedulePoint(tmp_list)
                self.schedule.append(tmp_point)
                schedin = schedin[13:]
                if len(schedin) > 0 and "assert" not in schedin[0]:
                    assert tmp_point.chosen in tmp_point.enabled
                    assert tmp_point.signalled in tmp_point.oncond
            elif "segfault" in item:
                self.error = item.strip()
                break
            elif "deadlock" in item:
                self.error = item.strip()
                break
            elif "thrille assert" in item:
                self.error = "THRILLE INTERNAL ERROR"
                break
            elif "program assert" in item:
                self.error = item.strip()
                break
            elif "end_log" in item:
                self.error = None
                break
            else:
                print "Unknown item:", item
                assert False
        assert len(self.getThreadOrder()) == (self.getContextSwitches() + 1)

    def outputExactSchedule(self, fileOut):
        fout = open(fileOut, "w")
        fout.write("begin_addr_list\n")
        for x in self.addrlist:
            fout.write(x + "\n")
        fout.write("end_addr_list\n")
        for x in self.schedule:
            if self.isSignal(x):
                fout.write("SIGNAL\n")
                fout.write("signalled:" + x.signalled + "\n")
                fout.write("caller:" + x.caller + "\n")
                fout.write("idaddr:" + x.addr + "\n")
                fout.write("cond:python_gen\n")
                fout.write("oncond:")
                for thr in x.oncond:
                    fout.write(thr + ",")
                fout.write("\n")
                if x.chosen is None:
                    continue
            
            fout.write("SCHED\n")
            fout.write("chosen:" + x.chosen + "\n")
            fout.write("caller:" + x.caller + "\n")
            fout.write("typstr:" + x.type + "\n")
            fout.write("idaddr:" + x.addr + "\n")
            fout.write("memor1:" + x.memory_1 + "\n")
            fout.write("memor2:" + x.memory_2 + "\n")
            fout.write("enable:")
            for thr in x.enabled:
                fout.write(thr + ",")
            fout.write("\n")

        fout.write("end_log")
        fout.close()

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

        i = 0
        times_scheduled = 0
        addr_dict = {}
        signal_list = []

        while i < len(self.schedule):
            tmp = self.schedule[i]
            if not addr_dict.has_key(tmp.addr):
                addr_dict[tmp.addr] = 0

            addr_dict[tmp.addr] += 1
            times_scheduled += 1
            if self.isSignal(tmp):
                signal_list.append(tmp.signalled)

            if tmp.chosen != tmp.caller:
                addr_dict[tmp.addr] -= 1
                times_scheduled -= 1

                self.outputScheduleStep(fout, tmp.caller, \
                        times_scheduled, tmp.addr, addr_dict[tmp.addr], \
                        signal_list)

                times_scheduled = 1
                addr_dict = {}
                signal_list = []

            i += 1

        self.outputScheduleStep(fout, self.schedule[-1].chosen, \
                 times_scheduled, "0", "0", signal_list) 

    def outputStrictSchedule(self, file_out):
        assert len(self.schedule) > 0
        assert False

    def getThreadOrder(self):
        if len(self.schedule) == 0:
            return []
        thread_order = []
        thread_order.append(self.schedule[0].caller)
        for x in self.schedule:
            if x.chosen != x.caller:
                thread_order.append(x.chosen)

        return thread_order

    def isSignal(self, sched):
        return hasattr(sched, "signalled")

    def isCheckpoint(self, sched_point):
        return sched_point.isCheckpoint()

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
            return Schedule()
        if j == i:
            return Schedule()

        assert j < i

        copy_schedule = copy.deepcopy(self.schedule)

        copy_front = copy_schedule[:j+1]
        copy_back = copy_schedule[i+1:]
        if len(copy_back) > 0:
            copy_back[0].addr = copy_schedule[j+1].addr

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
        sched, tei = self.consolidateFrontierForwardTEI(frontier)
        return sched
            
    def consolidateFrontierForwardTEI(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        if frontier + 1 >= len(self.schedule):
            return Schedule(), []
        
        target = self.schedule[frontier].caller


        if target == self.schedule[frontier].chosen:
            return Schedule(), []
        if target not in self.schedule[frontier].enabled:
            return Schedule(), []
        
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
            return Schedule(), []

        copy_schedule = copy.deepcopy(self.schedule)

        
        prefrontier = copy_schedule[:frontier]
        postfrontier = copy_schedule[frontier: block_start]
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

        return final_sched, mobileblock

    def getPrefixBeforeCheckpoint(self):
        assert self.getNumberOfCheckpoints() > 0
        prefix_sched = Schedule()
        prefix_sched.error = None
        prefix_sched.addrlist = self.addrlist

        for i in range(0, len(self.schedule)):
            sched_point = copy.deepcopy(self.schedule[i])
            prefix_sched.schedule.append(sched_point)
            if sched_point.isCheckpoint():
                break

        return prefix_sched





    def _consolidateActionForward(self, index):
        assert len(self.schedule) > 0
        assert index < len(self.schedule)
        assert index >= 0
        assert index + 1 < len(self.schedule)

        target = self.schedule[index].caller

        
        assert target != self.schedule[index].chosen
        #assert target in self.schedule[index].enabled
        
        search_ptr = index + 1

        while search_ptr < len(self.schedule):
            tmp_chosen = self.schedule[search_ptr].chosen
            if tmp_chosen == target:
                if (search_ptr + 1) == len(self.schedule):
                    tmp = self.schedule.pop(search_ptr)
                    self.schedule.insert(index + 1, tmp)
                    tmpc = self.schedule[index].chosen
                    self.schedule[index].chosen = self.schedule[index+1].chosen
                    self.schedule[index + 1].chosen = tmpc
                    self.schedule[index + 1].addr = "0"
                    break
                if self.schedule[search_ptr + 1].chosen == target:
                    tmp = self.schedule.pop(search_ptr + 1)
                    self.schedule.insert(index + 1, tmp)
                    tmpc = self.schedule[index].chosen
                    self.schedule[index].chosen = self.schedule[index+1].chosen
                    self.schedule[index + 1].chosen = tmpc
                    break
                else:
                    addr_save = self.schedule[search_ptr + 1].addr
                    self.schedule[search_ptr + 1].addr = self.schedule[search_ptr].addr
                    tmp = self.schedule.pop(search_ptr)
                    self.schedule.insert(index + 1, tmp)
                    tmpc = self.schedule[index].chosen
                    self.schedule[index].chosen = self.schedule[index+1].chosen
                    self.schedule[index + 1].chosen = tmpc
                    self.schedule[index + 1].addr = addr_save
                    break
            search_ptr += 1



    def synthesizeForwardSchedule(self, original, mobileTEI):
        copy_orig = copy.deepcopy(original)
        i = 0
        to_move = 0
        start_index = len(copy_orig.schedule)
        while i < len(copy_orig.schedule):
            if i < len(self.schedule):
                if self.schedule[i].chosen != copy_orig.schedule[i].chosen:
                    if start_index > i:
                        start_index = i
                    if (i - start_index) >= len(mobileTEI):
                        # control flow changed but we executed more actions
                        # than we moved, consider this a success
                        # and move up whole TEI
                        break;
                    tei_thr = mobileTEI[i - start_index].chosen
                    assert self.schedule[i].chosen  == tei_thr
                    to_move += 1
            i += 1

        while to_move > 0:
            try:
                copy_orig._consolidateActionForward(start_index)
            except AssertionError:
                pass
            copy_orig.patchUpSchedule(copy_orig.schedule)
            start_index += 1
            to_move -= 1


        final_sched = Schedule()
        final_sched.schedule = copy_orig.schedule
        final_sched.error = None
        final_sched.addrlist = copy_orig.addrlist

        return final_sched



    def consolidateFrontierBackward(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        if frontier == 0:
            return Schedule()

        target = self.schedule[frontier].chosen

        if target == self.schedule[frontier - 1].chosen:
            return Schedule()

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
            return Schedule()

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

    def blockExtend(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        target = self.schedule[frontier].caller

        if target == self.schedule[frontier].chosen:
            return Schedule()
        if target not in self.schedule[frontier].enabled:
            return Schedule() 
        
        block_start = frontier
        block_end = block_start
        copy_schedule = copy.deepcopy(self.schedule)

        tmp_start = copy_schedule[block_start].chosen
        assert tmp_start != target
        while tmp_start == copy_schedule[block_end].chosen:
            copy_schedule[block_end].chosen = target
            block_end += 1
            if block_end >= len(copy_schedule):
                break

        self.patchUpSchedule(copy_schedule)

        assert len(copy_schedule) == len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = copy_schedule
        final_sched.error = None
        final_sched.addrlist = self.addrlist

        return final_sched

    def preemptExtendSimple(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        target = self.schedule[frontier]

        if target.caller == target.chosen:
            return Schedule()
        if target.caller not in target.enabled:
            return Schedule()

        copy_schedule = copy.deepcopy(self.schedule)
        copy_schedule[frontier].addr = "0x10"
        
        assert len(copy_schedule) >= len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = copy_schedule
        final_sched.error = None
        final_sched.addrlist = self.addrlist

        return final_sched

    def preemptExtend(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        target = self.schedule[frontier].caller

        if target == self.schedule[frontier].chosen:
            return Schedule()
        if target not in self.schedule[frontier].enabled:
            return Schedule() 

        tmp = ["chosen:" + target, "caller:0", "typstr:Preempt Extend"] 
        tmp.append("idaddr:0x0")
        tmp.append("enabled:" + target)
        s = SchedulePoint(tmp)
        i = 0
        mid_schedule = [] 
        while i < 100:
            mid_schedule.append(copy.deepcopy(s))
            i += 1


        copy_schedule = copy.deepcopy(self.schedule)
        begin_schedule = copy_schedule[:frontier+1]
        end_schedule = copy_schedule[frontier+1:]
        copy_schedule = begin_schedule + mid_schedule + end_schedule
        self.patchUpSchedule(copy_schedule)

        assert len(copy_schedule) >= len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = copy_schedule
        final_sched.error = None
        final_sched.addrlist = self.addrlist

        return final_sched

    def blockExtendEqualButOpposite(self, frontier):
        assert len(self.schedule) > 0
        assert frontier < len(self.schedule)
        assert frontier >= 0

        target = self.schedule[frontier].caller

        if target == self.schedule[frontier].chosen:
            return Schedule()
        if target not in self.schedule[frontier].enabled:
            return Schedule() 
        
        block_start = frontier
        block_end = block_start
        copy_schedule = copy.deepcopy(self.schedule)

        tmp_start = copy_schedule[block_start].chosen
        assert tmp_start != target
        while tmp_start == copy_schedule[block_end].chosen:
            copy_schedule[block_end].chosen = target
            block_end += 1
            if block_end >= len(copy_schedule):
                break

        if block_end < len(copy_schedule):
            while copy_schedule[block_end].chosen != target:
                block_end += 1
                if block_end >= len(copy_schedule):
                    break

        if block_end < len(copy_schedule):
            while copy_schedule[block_end].chosen == target:
                copy_schedule[block_end].chosen = tmp_start
                block_end += 1
                if block_end >= len(copy_schedule):
                    break;

        self.patchUpSchedule(copy_schedule)

        assert len(copy_schedule) == len(self.schedule)

        final_sched = Schedule()
        final_sched.schedule = copy_schedule
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



def testOutputExactSchedule():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)

    test15 = os.path.join(td, "15.sched")
    assert os.path.exists(test15)
    s = Schedule(test15)
    testOut = os.path.join(td, "15.testout")
    try:
        os.remove(testOut)
    except:
        pass

    s.outputExactSchedule(testOut)

    testIn = open(testOut, "r").readlines()

    assert len(testIn) == 22

    assert testIn[0] == "begin_addr_list\n"
    assert testIn[1] == "0x100\n"
    assert testIn[2] == "end_addr_list\n"
    assert testIn[3] == "SCHED\n"
    assert testIn[4] == "chosen:1\n"
    assert testIn[5] == "caller:3\n"
    assert testIn[6] == "typstr:Before_Mutex_Lock\n"
    assert testIn[7] == "idaddr:0x400b0a\n"
    assert testIn[8] == "enable:1,3,\n"
    assert testIn[9] == "SIGNAL\n"
    assert testIn[10] == "signalled:0\n"
    assert testIn[11] == "caller:1\n"
    assert testIn[12] == "idaddr:0x400b4a\n"
    assert testIn[13] == "cond:python_gen\n"
    assert testIn[14] == "oncond:0,\n"
    assert testIn[15] == "SCHED\n"
    assert testIn[16] == "chosen:1\n"
    assert testIn[17] == "caller:1\n"
    assert testIn[18] == "typstr:Simulate_Signal\n"
    assert testIn[19] == "idaddr:0x400b4a\n"
    assert testIn[20] == "enable:1,0,\n"
    assert testIn[21] == "end_log"

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
    assert s.error == "thread 0 segfault" 
    assert s.addrlist == ["all"]
    assert s.schedule[1].caller == "1"
    assert s.schedule[0].type == "Before_Mutex_Lock"


    
    test3 = os.path.join(td, "03.sched")
    assert os.path.exists(test3)
    s = Schedule(test3)
    assert s.error == "program assert fail [1] 0x400"

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
    assert tmp != Schedule()
    assert len(tmp.schedule) == 6
    assert tmp.schedule[0].chosen == "2"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "0"
    assert tmp.schedule[3].chosen == "1"
    assert tmp.schedule[4].chosen == "1"
    assert tmp.schedule[5].chosen == "1"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "2", "0", "1"]
    assert tmp.schedule[-1].addr == "0x400b1b"

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("1")
    assert tmp != Schedule()
    assert len(tmp.schedule) == 4
    assert tmp.schedule[0].chosen == "2"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "0"
    assert tmp.schedule[3].chosen == "0"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "2",  "0"]
    assert tmp.schedule[-1].addr == "0x400be7"

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("2")
    assert tmp != Schedule()
    assert len(tmp.schedule) == 6
    assert tmp.schedule[0].chosen == "0"
    assert tmp.schedule[1].chosen == "0"
    assert tmp.schedule[2].chosen == "1"
    assert tmp.schedule[3].chosen == "1"
    assert tmp.schedule[4].chosen == "1"
    assert tmp.schedule[5].chosen == "0"
    thro = tmp.getThreadOrder()
    assert thro == ["0", "1", "0"]
    assert tmp.schedule[0].addr == s.schedule[0].addr
    assert tmp.schedule[2].addr == s.schedule[3].addr

    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.removeFinalBlockOfThread("3")
    assert tmp == Schedule()
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

def testIsCheckpoint():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test17 = os.path.join(td, "17.sched")
    assert os.path.exists(test17)
    s = Schedule(test17)

    assert s.getNumberOfCheckpoints() == 1

    assert not s.isCheckpoint(s.schedule[0])
    assert not s.isCheckpoint(s.schedule[1])
    assert not s.isCheckpoint(s.schedule[2])
    assert not s.isCheckpoint(s.schedule[3])
    assert s.isCheckpoint(s.schedule[4])
    assert not s.isCheckpoint(s.schedule[5])
    assert not s.isCheckpoint(s.schedule[6])
    return 1

def testConsolidateForward():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    
    #sched = [2, 0 ,0, 1, 1, 1, 0]
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(0)
    assert newsched != Schedule()
    assert newsched.getThreadOrder() == ["0", "2", "1", "0"]
    assert len(newsched.schedule) == 7
    assert newsched.schedule[0].chosen == "0"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "2"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "0"
    assert newsched.schedule[2].addr == s.schedule[3].addr
    assert newsched.schedule[3].addr == s.schedule[1].addr


    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(1)
    assert newsched == Schedule()

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(2)
    assert newsched == Schedule()

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(3)
    assert newsched != Schedule()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1"]
    assert len(newsched.schedule) == 7
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "0"
    assert newsched.schedule[4].chosen == "1"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "1"
    assert newsched.schedule[6].addr == s.schedule[5].addr
    assert newsched.schedule[4].addr == "0"


    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(4)
    assert newsched == Schedule()

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(5)
    assert newsched == Schedule()

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(6)
    assert newsched == Schedule()

    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    newsched = s.consolidateFrontierForward(5)
    assert newsched != Schedule()
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
    assert newsched.schedule[6].addr == s.schedule[7].addr
    assert newsched.schedule[7].addr == s.schedule[6].addr

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
    assert newsched != Schedule()
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
    assert newsched.schedule[5].addr == s.schedule[7].addr
    assert newsched.schedule[1].addr == s.schedule[1].addr

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched != Schedule()
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
    assert newsched.schedule[5].addr == s.schedule[6].addr
    assert newsched.schedule[4].addr == s.schedule[4].addr
   
    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched != Schedule()
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
    assert newsched.schedule[4].addr == s.schedule[5].addr
    assert newsched.schedule[3].addr == s.schedule[3].addr

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched == Schedule()

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched == Schedule()

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched == Schedule()

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched == Schedule()

    length -= 1
    newsched = s.consolidateFrontierBackward(length)
    assert newsched == Schedule()

    try:
        length -= 1
        newsched = s.consolidateFrontierBackward(length)
    except:
        pass
    else:
        assert False

    return 1


def testBlockExtend():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test14 = os.path.join(td, "14.sched")
    assert os.path.exists(test14)


    #frontier = 0
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,0,0,0,1,3,1,3,0
    s = Schedule(test14)
    length = 0
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "1", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "0"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"

    #frontier = 1
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,2,2,1,3,1,3,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "1", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "2"
    assert newsched.schedule[2].chosen == "2"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"
    
    #frontier = 2
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,1,3,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched == Schedule()

    #frontier = 3
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,0,3,1,3,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "0"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"

    #frontier = 4
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,1,3,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched == Schedule()

    #frontier = 5
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,3,3,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "3"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"

    #frontier = 6
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,1,1,0
    length += 1
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "3", "1", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "1"
    assert newsched.schedule[7].chosen == "0"

    #frontier = 7
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,1,3,3
    length += 1
    newsched = s.blockExtend(length)
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1", "3", "1", "3"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "2"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "0"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "3"

    #frontier = 8
    #start : 0,2,0,0,1,3,1,3,0
    #end   : 0,2,0,0,1,3,1,3,0
    length += 1
    try:
        newsched = s.blockExtend(length)
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
    assert newsched != Schedule()
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
    newsched = s.consolidateFrontierForward(3)
    assert newsched != Schedule()
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
    assert tmp != Schedule()
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
    newsched = s.consolidateFrontierForward(3)
    assert newsched != Schedule()
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
    assert newsched != Schedule()
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


def testFailureSix():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    length = s.getScheduleLength() - 1
    assert length == 7
    assert s.getThreadOrder() == ["0", "2", "0", "1", "3", "1", "3", "0"]
    newsched = s.consolidateFrontierForward(0)
    
    # thread order = [0, 2, 0, 1, 3, 1, 3, 0] 
    assert newsched != Schedule()
    assert newsched.getScheduleLength() == s.getScheduleLength()
    assert newsched.getThreadOrder() == ["0", "2", "1", "3", "1", "3", "0"]
    assert newsched.getScheduleLength() == 8
    assert newsched.schedule[0].chosen == "0"
    assert newsched.schedule[1].chosen == "0"
    assert newsched.schedule[2].chosen == "2"
    assert newsched.schedule[3].chosen == "1"
    assert newsched.schedule[4].chosen == "3"
    assert newsched.schedule[5].chosen == "1"
    assert newsched.schedule[6].chosen == "3"
    assert newsched.schedule[7].chosen == "0"
    return 1

def testFailureSeven():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")

    assert os.path.exists(td)
    test16 = os.path.join(td, "16.sched")
    assert os.path.exists(test16)
    s = Schedule(test16)
    assert s.schedule[40].addr == "0x404e94"
    assert s.schedule[47].addr == "0x404760"
    tmp = s.removeFinalBlockOfThread("2")
    assert tmp != Schedule()
    assert tmp.schedule[40].addr == "0x404e94"
    return 1
    

def testGetNonPreemptiveContextSwitches():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getNonPreemptiveContextSwitches() == 1
    s.schedule[0].enabled = ["2"]
    assert s.getNonPreemptiveContextSwitches() == 2
    return 1

def testGetAvgThreadsEnabledAtCS():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getAvgThreadsEnabledAtCS() == 9.0/4.0
    s.schedule[6].enabled = ["0", "1", "2", "3", "4"]
    assert s.getAvgThreadsEnabledAtCS() == 3
    s.schedule[2].enabled = ["0", "1", "2", "3", "4", "5"]
    assert s.getAvgThreadsEnabledAtCS() == 3
    return 1
   
def testGetAvgThreadsEnabledAtPreempts():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getAvgThreadsEnabledAtPreempts() == 7.0/3.0
    s.schedule[6].enabled = ["0", "1", "2", "3"]
    assert s.getAvgThreadsEnabledAtPreempts() == 3
    s.schedule[2].enabled = ["0", "1", "2", "3", "4", "5"]
    assert s.getAvgThreadsEnabledAtPreempts() == 3
    s.schedule[1].enabled = ["0", "1", "3", "4", "5", "6", "7", "8"]
    assert s.getAvgThreadsEnabledAtPreempts() == 3
    return 1
        
def testGetAvgThreadsEnabledAtNons():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getAvgThreadsEnabledAtNons() == 2.0
    s.schedule[6].enabled = ["0", "2", "3", "4", "5", "8"]
    assert s.getAvgThreadsEnabledAtNons() == 4.0
    s.schedule[2].enabled = ["0", "2", "3", "4", "5", "6"]
    assert s.getAvgThreadsEnabledAtNons() == 4.0
    s.schedule[0].enabled = ["0", "1", "2", "3", "4", "5", "6", "7", "8"]
    assert s.getAvgThreadsEnabledAtNons() == 4.0
    return 1

def testGetAvgThreadsEnabledAtAll():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    assert s.getAvgThreadsEnabledAtAll() == 13.0/7.0
    s.schedule[6].enabled = ["0", "2", "3"]
    assert s.getAvgThreadsEnabledAtAll() == 2.0
    return 1

def testIsInLastTEI():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    
    #sched = [2, 0 ,0, 1, 1, 1, 0]
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(0)
    assert newsched != Schedule()
    assert len(newsched.schedule) == 7
    assert newsched.isInLastTEI(0) == False
    assert newsched.isInLastTEI(1) == False
    assert newsched.isInLastTEI(2) == False
    assert newsched.isInLastTEI(3) == False
    assert newsched.isInLastTEI(4) == False
    assert newsched.isInLastTEI(5) == False
    assert newsched.isInLastTEI(6) == True

    s = Schedule(test5)
    newsched = s.consolidateFrontierForward(3)
    assert newsched != Schedule()
    assert len(newsched.schedule) == 7
    assert newsched.isInLastTEI(0) == False
    assert newsched.isInLastTEI(1) == False
    assert newsched.isInLastTEI(2) == False
    assert newsched.isInLastTEI(3) == False
    assert newsched.isInLastTEI(4) == True
    assert newsched.isInLastTEI(5) == True 
    assert newsched.isInLastTEI(6) == True 


    assert os.path.exists(td)
    test9 = os.path.join(td, "09.sched")
    assert os.path.exists(test9)
    s = Schedule(test9)
    newsched = s.consolidateFrontierForward(5)
    assert newsched != Schedule()
    assert len(newsched.schedule) == 8
    assert newsched.isInLastTEI(0) == False
    assert newsched.isInLastTEI(1) == False
    assert newsched.isInLastTEI(2) == False
    assert newsched.isInLastTEI(3) == False
    assert newsched.isInLastTEI(4) == False 
    assert newsched.isInLastTEI(5) == False 
    assert newsched.isInLastTEI(6) == False
    assert newsched.isInLastTEI(7) == True 
    
    return 1

def testSynthesizeForwardSchedule():
    tr = os.environ.get("THRILLE_ROOT")
    assert tr is not None
    td = os.path.join(tr, "tests", "fwdrev")
    
    #sched = [2, 0 ,0, 1, 1, 1, 0]
    assert os.path.exists(td)
    test5 = os.path.join(td, "05.sched")
    assert os.path.exists(test5)
    s = Schedule(test5)
    tmp = s.schedule[2]
    s.schedule[2] = s.schedule[5]
    s.schedule[5] = tmp
    s.patchUpSchedule(s.schedule)
    s.schedule[2].enabled.append("0")
    newsched, tei = s.consolidateFrontierForwardTEI(2)
    assert newsched != Schedule()
    assert newsched.getThreadOrder() == ["0", "2", "0", "1"]
    assert len(newsched.schedule) == 7
    
    newsched.schedule = newsched.schedule[:3]
    result = newsched.synthesizeForwardSchedule(s, tei)
    assert len(result.schedule) == 7
    assert result.schedule[0].chosen == "2"
    assert result.schedule[1].chosen == "0"
    assert result.schedule[2].chosen == "0"
    assert result.schedule[3].chosen == "1"
    assert result.schedule[4].chosen == "1"
    assert result.schedule[5].chosen == "1"
    assert result.schedule[6].chosen == "0"
    assert result.schedule[6].addr == s.schedule[5].addr
    assert result.schedule[3].addr == s.schedule[6].addr


    newsched, tei = s.consolidateFrontierForwardTEI(2)
    newsched.schedule = newsched.schedule[:4]
    result = newsched.synthesizeForwardSchedule(s, tei)

    assert len(result.schedule) == 7
    assert result.schedule[0].chosen == "2"
    assert result.schedule[1].chosen == "0"
    assert result.schedule[2].chosen == "0"
    assert result.schedule[3].chosen == "0"
    assert result.schedule[4].chosen == "1"
    assert result.schedule[5].chosen == "1"
    assert result.schedule[6].chosen == "1"
    assert result.schedule[6].addr == s.schedule[4].addr
    assert result.schedule[4].addr == "0"



    return 1


def test():
    test_passed = 0

    print "schedule.py:"

    test_passed += testOutputExactSchedule()
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
    test_passed += testIsCheckpoint()
    test_passed += testConsolidateForward()
    test_passed += testConsolidateReverse()
    test_passed += testBlockExtend()
    test_passed += testFailureOne()
    test_passed += testFailureTwo()
    test_passed += testFailureThree()
    test_passed += testFailureFour()
    test_passed += testFailureFive()
    test_passed += testFailureSix()
    test_passed += testFailureSeven()
    test_passed += testGetNonPreemptiveContextSwitches()
    test_passed += testGetAvgThreadsEnabledAtCS()
    test_passed += testGetAvgThreadsEnabledAtPreempts()
    test_passed += testGetAvgThreadsEnabledAtNons()
    test_passed += testGetAvgThreadsEnabledAtAll()
    test_passed += testIsInLastTEI()
    test_passed += testSynthesizeForwardSchedule()

    print "Tests passed:", test_passed



if __name__ == "__main__":
    test()

