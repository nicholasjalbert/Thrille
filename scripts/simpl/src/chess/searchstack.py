# Implementation of search stack for DFS stateless model checking
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal Matter>

import os
import sys
import copy
import subprocess
import shutil

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule

class SearchStackElement:
    def __init__(self, sched_point, issignal):
        self.schedule_point = sched_point
        self.issignal = issignal
        self.ischeckpoint = sched_point.isCheckpoint()
        self.done = []
        if self.issignal:
            assert self.schedule_point.isSignal()
            self.chosen = copy.deepcopy(self.schedule_point.signalled)
            self.todo = copy.deepcopy(self.schedule_point.oncond)
        else:
            self.chosen = copy.deepcopy(self.schedule_point.chosen)
            self.todo = copy.deepcopy(self.schedule_point.enabled)

        assert self.chosen in self.todo
        self.todo.remove(self.chosen)
        self.done.append(self.chosen)

    
    def __repr__(self):
        my_rep = "SearchStackElement("
        my_rep += "\n\t\t\tchosen: " + str(self.chosen)
        my_rep += "\n\t\t\tpoint:" + str(self.schedule_point)
        my_rep += "\n\t\t\tdone: " + str(self.done) 
        my_rep += "\n\t\t\ttodo: " + str(self.todo)
        my_rep += "\n\t\t\tis_signal: " + str(self.issignal)
        my_rep += ")"
        return my_rep

    def dbgRepr(self):
        my_rep = "[chosen: " + str(self.chosen)
        my_rep += ", done: " + str(self.done) 
        my_rep += ", todo: " + str(self.todo)
        my_rep += ", is_signal: " + str(self.issignal) + "]"
        return my_rep

    def doWork(self):
        assert len(self.todo) > 0
        self.chosenIsDone()
        self.chosen = self.todo.pop()
        if self.isSignal():
            assert self.chosen in self.schedule_point.oncond
        else:
            assert self.chosen in self.schedule_point.enabled
    
    def needPreemptsToExplore(self):
        if self.issignal:
            return False
        else:
            if self.schedule_point.caller in self.schedule_point.enabled:
                if self.schedule_point.caller == self.chosen:
                    return True
                else:
                    return False
            else:
                return False

    def hasToDo(self):
        return len(self.todo) > 0

    def isPreempt(self):
        if self.issignal:
            return False
        else:
            if self.schedule_point.caller in self.schedule_point.enabled:
                if self.schedule_point.caller != self.chosen:
                    return True
                else:
                    return False
            else:
                return False

    def makeSchedulePoint(self):
        ret_point = copy.deepcopy(self.schedule_point)
        if self.isSignal():
            ret_point.signalled = self.chosen
        else:
            ret_point.chosen = self.chosen
        return ret_point

    def isSignal(self):
        return self.issignal

    def isCheckpoint(self):
        return self.ischeckpoint

    def chosenIsDone(self):
        if not (self.chosen in self.done):
            self.done.append(self.chosen)

class ChessSearchStack:
    def __init__(self, bound, addrlist):
        self.preempt_bound = int(bound)
        self.current_preempts = 0
        self.addrlist = addrlist
        self.stack = []

    def __repr__(self):
        my_rep = "SearchStack(\n"
        my_rep += "\tpreempt_bound: " + str(self.preempt_bound) + "\n"
        my_rep += "\tcurrent_preempt: " + str(self.current_preempts) + "\n"
        my_rep += "\tstack:\n"
        for el in self.stack:
            my_rep += "\t\t" + str(el) + "\n"
        my_rep += ")"
        return my_rep

    def __len__(self):
        return len(self.stack)

    def calculatePreemptionsInRange(self, start):
        calc_range = range(start, len(self.stack))
        preempt_list = [i for i in calc_range if self.stack[i].isPreempt()]
        return len(preempt_list)

    def calculatePreemptions(self):
        preempts = self.calculatePreemptionsInRange(0)
        self.current_preempts = preempts
        assert self.current_preempts <= self.preempt_bound

    def getDebugRepresentation(self, size):
        my_dbg = "Search Stack\n"
        my_dbg += "Preemption Bound: " + str(self.preempt_bound) + "\n"
        my_dbg += "Current Preemptions: " + str(self.current_preempts) + "\n"
        my_dbg += "Stack:\n"
       
        x = 0
        if len(self) > size:
            my_dbg += "\t" + str(len(self) - size) + " more elements...\n"

        if size > len(self):
            x = -1 * len(self)
        else:
            x = -1 * size

        while x < 0:
            my_dbg += self.stack[x].dbgRepr() + " cost:" 
            if self.needPreemptsToExplore(self.stack[x]):
                my_dbg += " 1\n"
            else:
                my_dbg += " 0\n"
            x += 1
        return my_dbg

    def push(self, element):
        self.stack.append(element)
        self.calculatePreemptions()
        assert self.current_preempts <= self.preempt_bound

    def pop(self):
        el = self.stack.pop()
        self.calculatePreemptions()
        return el

    def popBottom(self):
        el = self.stack.pop(0)
        self.calculatePreemptions()
        return el

    def isSearchComplete(self):
        return len(self.stack) == 0

    def needPreemptsToExplore(self, element):
        return element.needPreemptsToExplore()


    def isWorkToDo(self, element):
        if not element.hasToDo():
            return False
        if self.current_preempts < self.preempt_bound:
            return True
        elif self.current_preempts == self.preempt_bound:
            if self.needPreemptsToExplore(element):
                return False
            else:
                return True
        else:
            assert False

    def buildSchedule(self):
        return_schedule = schedule.Schedule()
        return_schedule.error = None
        return_schedule.addrlist = self.addrlist
        tmp_sched = [x.makeSchedulePoint() for x in self.stack]
        while len(tmp_sched) > 0:
            elem = tmp_sched.pop(0)
            if elem.isSignal():
                if len(tmp_sched) > 0:
                    following_elem = tmp_sched.pop(0)
                    assert following_elem.signalled == elem.signalled
                    assert following_elem.caller == elem.caller
                    elem = following_elem
                else:
                    elem.chosen = None
            return_schedule.schedule.append(elem)
        return return_schedule

    def doWork(self, element):
        element.doWork()

    def generateNextSchedule(self):
        while not self.isSearchComplete():
            element = self.pop()
            if self.isWorkToDo(element): 
                self.doWork(element)
                self.push(element)
                return self.buildSchedule()
        return None

    def mergeWith(self, other_search_stack):
        while len(other_search_stack) > 0:
            tmp = other_search_stack.popBottom()
            self.push(tmp)

    def stackTopIsDone(self):
        self.stack[-1].chosenIsDone()

    def getEmptySearchStack(self, bound, addrlist):
        return ChessSearchStack(bound, addrlist)
    
    def generateSearchStack(self, bound, schedule):
        assert schedule.addrlist == self.addrlist
        my_searchstack = self.getEmptySearchStack(bound, self.addrlist)

        for sched_point in schedule.schedule:
            if sched_point.isSignal():
                tmp = SearchStackElement(sched_point, True)
                my_searchstack.push(tmp)
            tmp = SearchStackElement(sched_point, False) 
            my_searchstack.push(tmp)
        return my_searchstack

    # Signals get split into two elements on the stack: 1 for who to
    #      signal, and 1 for who to schedule after the signal.  They are
    #      stored as a single unit in a schedule though, and this checks
    #      to make sure we aren't losing any information.
    def checkForSplitSignal(self, schedule):
        if schedule.getScheduleLength() > 0:
            if hasattr(schedule.schedule[0], "signalled"):
                if schedule.schedule[0].signalled is None:
                    last_el = self.pop()
                    assert last_el.isSignal()
                    tmp = schedule.schedule.pop(0)
                    assert tmp.signalled is  None
                    tmp.signalled = last_el.chosen
                    stack_el = SearchStackElement(tmp, False) 
                    self.push(last_el)
                    self.push(stack_el)


    def regenerate(self, target_sched, result_sched):
        postfix = result_sched.getPostfixAfter(target_sched)
        self.stackTopIsDone()
        self.checkForSplitSignal(postfix)
        tmp_stack = self.generateSearchStack(0, postfix)
        self.mergeWith(tmp_stack)

class ChessCheckpointedSearchStack(ChessSearchStack):
    
    def __init__(self, bound, addrlist):
        ChessSearchStack.__init__(self, bound, addrlist)
        print "Chess Checkpointed Search Beginning"
    
    def calculatePreemptions(self):
        self.calculatePreemptionsFromDeepestCheckpoint()

    def calculatePreemptionsFromDeepestCheckpoint(self):
        # find position in search stack of the deepest checkpoint
        deepestCheckpoint = len(self.stack) - 1
        if deepestCheckpoint == -1:
            deepestCheckpoint = 0
        while deepestCheckpoint > 0:
            if self.stack[deepestCheckpoint].isCheckpoint():
                break
            deepestCheckpoint -= 1
        
        # if no checkpoint, this search will terminate
        if deepestCheckpoint == 0:
            assert self.isSearchComplete()
            return
        
        # calculate preemptions from checkpoint to end only      
        preempts = self.calculatePreemptionsInRange(deepestCheckpoint)
        self.current_preempts = preempts
        assert self.current_preempts <= self.preempt_bound

    def getEmptySearchStack(self, bound, addrlist):
        return ChessCheckpointedSearchStack(bound, addrlist)
    
    # terminate when no checkpoint is on stack
    def isSearchComplete(self):
        for element in self.stack:
            if element.isCheckpoint():
                return False
        return True

    def getNumberOfCheckpoints(self):
        checkpoints = 0
        for element in self.stack:
            if element.isCheckpoint():
                checkpoints += 1
        return checkpoints

   
def testInit():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "01.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    
    test_el = SearchStackElement(test_sched.schedule[0], False)
    assert not test_el.issignal
    assert test_el.chosen == "2"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "2"
    assert len(test_el.todo) == 2
    assert test_el.todo[0] == "0"
    assert test_el.todo[1] == "1"
    
    test_el = SearchStackElement(test_sched.schedule[1], False)
    assert not test_el.issignal
    assert test_el.chosen == "0"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "0"
    assert len(test_el.todo) == 1
    assert test_el.todo[0] == "1"
    
    test_el = SearchStackElement(test_sched.schedule[2], True)
    assert test_el.issignal
    assert test_el.chosen == "3"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "3"
    assert len(test_el.todo) == 3
    assert test_el.todo[0] == "0"
    assert test_el.todo[1] == "2"
    assert test_el.todo[2] == "4"
    
    test_el = SearchStackElement(test_sched.schedule[2], False)
    assert not test_el.issignal
    assert test_el.chosen == "1"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "1"
    assert len(test_el.todo) == 0
    return 1
    
def testDoWork():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "01.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)

    test_el = SearchStackElement(test_sched.schedule[2], False)
    try:
        test_el.doWork()
    except AssertionError:
        pass
    else:
        assert False
    
    test_el = SearchStackElement(test_sched.schedule[2], True)
    test_el.doWork()
    assert test_el.chosen == "4"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "3"
    assert len(test_el.todo) == 2
    assert test_el.todo[0] == "0"
    assert test_el.todo[1] == "2"

    test_el = SearchStackElement(test_sched.schedule[1], False)
    test_el.doWork()
    assert test_el.chosen == "1"
    assert len(test_el.done) == 1
    assert test_el.done[0] == "0"
    assert len(test_el.todo) == 0

    return 1

    
def testNeedPreemptsToExplore():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "01.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)

    test_el = SearchStackElement(test_sched.schedule[2], True)
    assert not test_el.needPreemptsToExplore()
   
    test_el = SearchStackElement(test_sched.schedule[1], False)
    assert not test_el.needPreemptsToExplore()

    test_el = SearchStackElement(test_sched.schedule[0], False)
    assert not test_el.needPreemptsToExplore()
    test_el.doWork()
    test_el.chosen = "0"
    assert test_el.needPreemptsToExplore()

    return 1

def testIsPreempt():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "01.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    
    test_el = SearchStackElement(test_sched.schedule[0], False)
    assert test_el.isPreempt()
    
    test_el = SearchStackElement(test_sched.schedule[1], False)
    assert not test_el.isPreempt()
    
    test_el = SearchStackElement(test_sched.schedule[2], True)
    assert not test_el.isPreempt()
    
    test_el = SearchStackElement(test_sched.schedule[2], False)
    assert not test_el.isPreempt()
    return 1

def testChosenIsDone():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "01.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    
    test_el = SearchStackElement(test_sched.schedule[0], False)

    assert len(test_el.done) == 1
    test_el.chosenIsDone()
    assert len(test_el.done) == 1

    test_el.chosen = "0"
    assert "0" not in test_el.done
    test_el.chosenIsDone()
    assert "0" in test_el.done

    return 1

def testGenerateSearchStack():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)
    assert my_stack.preempt_bound == 1
    assert my_stack.current_preempts == 1
    assert my_stack.addrlist == ["0x100"]
    assert len(my_stack.stack) == 4
    return 1


def testCalculatePreemptions():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)
    
    my_stack.calculatePreemptions()
    assert my_stack.current_preempts == 1
    
    my_stack.stack[0].chosen = "0"
    my_stack.calculatePreemptions()
    assert my_stack.current_preempts == 0
   
    my_stack.stack[0].chosen = "2"
    my_stack.preempt_bound = 0
    try:
        my_stack.calculatePreemptions()
    except AssertionError:
        pass
    else:
        assert False

    return 1

def testIsWorkToDo():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)
    
    el = my_stack.pop()
    assert not my_stack.isWorkToDo(el)

    el = my_stack.pop()
    assert my_stack.isWorkToDo(el)

    el = my_stack.pop()
    assert my_stack.isWorkToDo(el)
    
    el = my_stack.pop()
    assert my_stack.isWorkToDo(el)

    return 1

def testBuildSchedule():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)

    my_sched = my_stack.buildSchedule()

    assert my_sched.addrlist == ["0x100"]
    assert my_sched.error is None
    assert len(my_sched.schedule) == 3

    assert my_sched.schedule[0].chosen == "2"
    assert my_sched.schedule[1].chosen == "1"
    assert my_sched.schedule[2].chosen == "1"
    assert my_sched.schedule[2].signalled == "3"

    return 1

def testGenerateNextSchedule():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = \
            ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)

    sched = my_stack.generateNextSchedule()
    assert sched.addrlist == ["0x100"]
    assert len(sched.schedule) == 3
    assert sched.schedule[2].signalled == "5"
    assert sched.schedule[2].chosen is None
    
    sched = my_stack.generateNextSchedule()
    assert sched.addrlist == ["0x100"]
    assert len(sched.schedule) == 3
    assert sched.schedule[2].signalled == "4"
    assert sched.schedule[2].chosen is None
    
    sched = my_stack.generateNextSchedule()
    assert sched.addrlist == ["0x100"]
    assert len(sched.schedule) == 2
    assert sched.schedule[1].chosen == "0"
    
    sched = my_stack.generateNextSchedule()
    assert sched.addrlist == ["0x100"]
    assert len(sched.schedule) == 1
    assert sched.schedule[0].chosen == "1"
    
    sched = my_stack.generateNextSchedule()
    assert sched.addrlist == ["0x100"]
    assert len(sched.schedule) == 1
    assert sched.schedule[0].chosen == "0"
    
    return 1

def testRegenerate():
    tr = os.environ.get('THRILLE_ROOT')
    test_src = os.path.join(tr, "tests", "fwdrev", "chess", "02.sched")
    assert os.path.exists(test_src)
    test_sched = schedule.Schedule(test_src)
    my_stack = \
            ChessSearchStack(0, ["0x100"]).generateSearchStack(1, test_sched)
    gen_sched = copy.deepcopy(test_sched)
    gen_sched.schedule.pop()
    gen_sched.schedule.pop()
    
    my_stack.pop()
    my_stack.pop()
    my_stack.pop()
    
    assert len(my_stack) == 1
    assert len(gen_sched.schedule) == 1

    my_stack.regenerate(gen_sched, test_sched)

    assert len(my_stack) == 4
    assert my_stack.stack[2].isSignal()
    assert my_stack.stack[3].chosen == "1"

    return 1


def test():
    test_passed = 0
    print "searchstack.py:"

    test_passed += testInit()
    test_passed += testDoWork()
    test_passed += testNeedPreemptsToExplore()
    test_passed += testIsPreempt()
    test_passed += testChosenIsDone()
    test_passed += testGenerateSearchStack()
    test_passed += testCalculatePreemptions()
    test_passed += testIsWorkToDo()
    test_passed += testBuildSchedule()
    test_passed += testGenerateNextSchedule()
    test_passed += testRegenerate()


    print "Tests passed:", test_passed

if __name__ == "__main__":
    test()
 
