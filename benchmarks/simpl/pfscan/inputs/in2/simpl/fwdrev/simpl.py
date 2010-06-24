# latest version of simplification algorithm (9/22/2009)
# iteratively runs:
#       -block removal
#       -forward consolidation
#       -backward consolidation
# until the schedule reaches a fixed point
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal Matter>

import os
import sys
import copy
import subprocess
import shutil
import fwdrev
import pickle

class Simplifier:
    def __init__(self, tr, input, bin, binflags):
        self.thrille_out = "my-schedule"
        self.thrille_in = "thrille-relaxed-sched"
        self.relaxedserial = os.path.join(tr, "bin", "librelaxedserial.so")
        self.tr = tr
        self.input = input
        self.bin = bin
        self.bindir, tmp = os.path.split(bin)
        if self.bindir == "":
            self.bindir = "."
        self.binflags = binflags
        self.start_schedule = fwdrev.Schedule(input)
        assert os.path.exists(self.relaxedserial)
        sanitycheck = self.testSchedule(self.start_schedule).error
        print sanitycheck
        print self.start_schedule.error
        assert  sanitycheck == self.start_schedule.error

    def testSchedule(self, sched):
        if sched is None:
            return fwdrev.Schedule()
        cwd = os.getcwd()
        os.chdir(self.bindir)
        output_schedule = os.path.join(self.bindir, self.thrille_in)
        sched.outputRelaxedSchedule(output_schedule)
        os.environ["LD_PRELOAD"] =  self.relaxedserial
        call_list = []
        call_list = [os.path.join(self.bindir, self.bin)] + self.binflags
        fout = open(os.path.join(self.bindir, "execution.log"), "w")
        exit = subprocess.call(call_list, stdout=fout, stderr=fout)
        #exit = subprocess.call(call_list)
        fout.close()
        del os.environ["LD_PRELOAD"]
        os.remove(os.path.join(self.bindir, "execution.log"))
        os.chdir(cwd)
        return fwdrev.Schedule(os.path.join(self.bindir, self.thrille_out))

    def sanityFail(self, untested_sched, tested_sched, sanity_sched):
        print "We thought the sched had error:", tested_sched.error
        print "It really had:", sanity_sched.error
        pickle.dump(untested_sched, open("foobar-untested",\
                "w"))
        untested_sched.outputRelaxedSchedule(\
                "foobar-relaxed-untested")
        fout = open("foobar-untested-print", "w")
        sys.stdout = fout
        print "Untested\n", untested_sched
        sys.stdout = sys.__stdout__
        fout.close()
        pickle.dump(tested_sched, open("foobar-tested", "w"))
        tested_sched.outputRelaxedSchedule(\
                "foobar-relaxed-tested")
        fout = open("foobar-tested-print", "w")
        sys.stdout = fout
        print "Tested\n", untested_sched
        sys.stdout = sys.__stdout__
        fout.close()
        pickle.dump(sanity_sched, open("foobar-sanity", "w"))
        sanity_sched.outputRelaxedSchedule(\
                "foobar-relaxed-sanity")
        fout = open("foobar-sanity-print", "w")
        sys.stdout = fout
        print "Sanity\n", sanity_sched
        sys.stdout = sys.__stdout__
        fout.close()
        assert False



    def blockRemove(self, sched):
        assert self.testSchedule(sched).error == sched.error
        threadOrder = sched.getThreadOrder()
        threadOrder.reverse()
        thread_done = {}
        return_sched = sched.cloneThyself()
        for x in threadOrder:
            if x not in thread_done:
                untested_sched = return_sched.removeFinalBlockOfThread(x)
                tested_sched = self.testSchedule(untested_sched)
                if tested_sched.error == return_sched.error:
                    return_sched = tested_sched
                else:
                    thread_done[x] = True
        assert self.testSchedule(return_sched).error == sched.error
        return return_sched

    def simplifyForward(self, sched):
        assert self.testSchedule(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.consolidateFrontierForward(i)
            tested_sched = self.testSchedule(untested_sched)
            if tested_sched.error == return_sched.error:
                return_sched = tested_sched
            i += 1
        assert self.testSchedule(return_sched).error == sched.error
        return return_sched
    
    def simplifyBackward(self, sched):
        assert self.testSchedule(sched).error == sched.error
        i = sched.getScheduleLength() - 1
        return_sched = sched.cloneThyself()
        assert return_sched.getScheduleLength() == (i + 1)
        while i > 0:
            # check if some change we made shortened the schedule
            # i.e. we hit the error earlier
            if i >= return_sched.getScheduleLength():
                i = (return_sched.getScheduleLength() - 1)
            untested_sched = return_sched.consolidateFrontierBackward(i)
            tested_sched = self.testSchedule(untested_sched)
            if tested_sched.error == return_sched.error:
                return_sched = tested_sched
            i -= 1
        assert self.testSchedule(return_sched).error == sched.error
        return return_sched

    def printEffect(self, string, oldsched, newsched):
        print "\t", string, ":"
        oldblock = oldsched.getScheduleLength()
        newblock = newsched.getScheduleLength()
        print "\t\tBlocks Removed:", oldblock - newblock
        oldthr = oldsched.getNumberOfThreads()
        newthr = newsched.getNumberOfThreads()
        print "\t\tThreads Removed:", oldthr - newthr
        oldctxt = oldsched.getContextSwitches()
        newctxt = newsched.getContextSwitches()
        print "\t\tContext Switches Removed:", oldctxt - newctxt
        oldpre = oldsched.getPreemptions()
        newpre = newsched.getPreemptions()
        print "\t\tPreemptions Removed:", oldpre - newpre

    def simplify(self, debug=True):
        if self.start_schedule.error is None:
            return self.start_schedule

        last_sched = None
        current_sched = self.start_schedule.cloneThyself()
        i = 0

        while current_sched != last_sched:
            if debug:
                print "Iteration", i, ":\n", 
                if i == 0:
                    current_sched_str = current_sched.getSummaryInfo()
                    print
                    print "Start Schedule Statistics"
                    print current_sched_str
                    print
            br_sched = self.blockRemove(current_sched)
            if debug:
                self.printEffect("Block Removal", current_sched, br_sched)
            for_sched = self.simplifyForward(br_sched)
            if debug:
                self.printEffect("Forward Consolidation", br_sched, for_sched)
            back_sched = self.simplifyBackward(for_sched)
            if debug:
                self.printEffect("Backward Consolidate", for_sched, back_sched)
            last_sched = current_sched
            current_sched = back_sched
            i += 1

        assert current_sched.error == self.start_schedule.error
        tmp_error = self.testSchedule(current_sched).error
        assert tmp_error == self.start_schedule.error
        return current_sched

    def getStartSched(self):
        return self.start_schedule



def checkEnvironment():
   
    if len(sys.argv) < 4:
        if len(sys.argv) == 2 and sys.argv[1] == "-test":
            assert os.environ.get('THRILLE_ROOT') != None
            return

        print "usage: python simpl.py  [input schedule] [output schedule]",
        print "[binary under test] [binary flags]"
        print
        print "testing: python simpl.py -test"
        print
        print "purpose: performs the block removal AND",
        print "preemption simplification algorithm (forward + backward)",
        print "saving the results to [output schedule]"
        print
        sys.exit(1)

    assert os.path.exists(sys.argv[1]), "Input does not exist"
    assert os.environ.get('THRILLE_ROOT') != None
    assert os.path.exists(sys.argv[3]), "Binary does not exist"

def testSetup(test):
    tr = os.environ.get('THRILLE_ROOT')
    td = os.path.join(tr, "tests", "fwdrev", "simpl")
    assert os.path.exists(td)
    test_src = os.path.join(td, test + ".cpp")
    assert os.path.exists(test_src)
    log = os.path.join(td, "logfile")
    bin = os.path.join(td, test)
    fout = open(log, "w")
    args = ["g++", "-lpthread", "-g", "-o", bin, test_src] 
    exit = subprocess.call(args, stdout=fout, stderr=fout)
    assert exit == 0
    assert os.path.exists(bin)
    fout.close()
    os.remove(log)
    return tr, td, bin

def testTeardown(test, tr, td, bin):
    os.remove(bin)
    os.remove(os.path.join(td, "my-schedule"))
    os.remove(os.path.join(td, "thrille-relaxed-sched"))


def testBlockRemove():
    tr, td, bin = testSetup("01")
    schedin = os.path.join(td, "01.sched1")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getNumberOfThreads() == 4
    news = s.blockRemove(s.start_schedule)
    assert news.getNumberOfThreads() == 4

    schedin = os.path.join(td, "01.sched2")
    s = Simplifier(tr, schedin, bin, binflags)
    assert s.start_schedule.error is None
    end_sched = s.blockRemove(s.start_schedule) 
    assert end_sched.getNumberOfThreads() == 3
    assert end_sched.getScheduleLength() == 60
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getNumberOfThreads() == 4
    news = s.blockRemove(s.start_schedule)
    assert news.getNumberOfThreads() == 2
    testTeardown("02", tr, td, bin)
    return 1

def testSimplifyForward():
    tr, td, bin = testSetup("01")
    schedin = os.path.join(td, "01.sched1")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 11
    assert s.start_schedule.getPreemptions() == 8
    news = s.simplifyForward(s.start_schedule)
    assert news.getContextSwitches() == 4
    assert news.getPreemptions() == 1
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 8
    assert s.start_schedule.getPreemptions() == 6
    brs = s.blockRemove(s.start_schedule)
    news = s.simplifyForward(brs)
    assert news.getContextSwitches() == 2
    assert news.getPreemptions() == 1

    news = s.simplifyForward(s.start_schedule)
    assert news.getContextSwitches() == 4
    assert news.getPreemptions() == 2
    testTeardown("02", tr, td, bin)

    return 1    

def testSimplifyBackward():
    tr, td, bin = testSetup("01")
    schedin = os.path.join(td, "01.sched1")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 11
    assert s.start_schedule.getPreemptions() == 8
    news = s.simplifyBackward(s.start_schedule)
    assert news.getContextSwitches() == 4
    assert news.getPreemptions() == 1
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    assert "assert" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 8
    assert s.start_schedule.getPreemptions() == 6
    brs = s.blockRemove(s.start_schedule)
    news = s.simplifyBackward(brs)
    assert news.getContextSwitches() == 2
    assert news.getPreemptions() == 1

    news = s.simplifyBackward(s.start_schedule)
    assert news.getContextSwitches() == 2
    assert news.getPreemptions() == 1
    assert news.getNumberOfThreads() == 2
    testTeardown("02", tr, td, bin)
    return 1    

def testSimplify():
    tr, td, bin = testSetup("01")
    fout = open(td + "simplify.log", "w")
    sys.stdout = fout
    schedin = os.path.join(td, "01.sched1")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    result = s.simplify()
    assert result.getNumberOfThreads() == 4
    assert result.getContextSwitches() == 4
    assert result.getPreemptions() == 1
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    s = Simplifier(tr, schedin, bin, binflags)
    result = s.simplify()
    assert result.getNumberOfThreads() == 2
    assert result.getContextSwitches() == 2
    assert result.getPreemptions() == 1
    testTeardown("02", tr, td, bin)
    sys.stdout = sys.__stdout__
    fout.close()
    os.remove(td + "simplify.log")
    return 1


def test():
    test_passed = 0
    tr = os.environ.get('THRILLE_ROOT')
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test_log = os.path.join(td, "simpltest.log")
    fout = open(test_log, "w")
    sys.stdout  = fout
    test_passed += testBlockRemove()
    test_passed += testSimplifyForward()
    test_passed += testSimplifyBackward()
    test_passed += testSimplify()
    sys.stdout = sys.__stdout__
    fout.close()
    os.remove(test_log)
    print "Tests Passed:", test_passed


def main():
    checkEnvironment()
    tr = os.environ.get('THRILLE_ROOT')
    input = sys.argv[1]
    if input == "-test":
        test()
        return
    output = sys.argv[2]
    bin = sys.argv[3]
    binflags = sys.argv[4:]
    s = Simplifier(tr, input, bin, binflags)
    done_sched = s.simplify()
    start_sched = s.getStartSched()
    start_str = start_sched.getSummaryInfo()
    done_str = done_sched.getSummaryInfo()
    print "Start Schedule:"
    print start_str
    print
    print "Simplified Schedule:"
    print done_str
    print
    done_sched.outputRelaxedSchedule(output)

if __name__ == "__main__":
    main()
