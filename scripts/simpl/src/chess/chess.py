# Implementation of context bounded modelchecking
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal Matter>

import os
import sys
import copy
import subprocess
import shutil
import searchstack
import pickle
import time 

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule

class ChessChecker:
    def __init__(self, tr, preemption_bound, addrlist, results, bin, binflags):
        self.result_dir = os.path.abspath(results)
        assert os.path.exists(self.result_dir)
        self.check_start_time = 0
        self.check_end_time = 0
        self.completed_search = False
        self.thrille_out = "my-schedule"
        self.thrille_in = "thrille-sched"
        self.npfthriller = os.path.join(tr, "bin", "libchessserial.so")
        assert os.path.exists(self.npfthriller)
        self.executions = 0
        self.tr = tr
        assert os.path.exists(bin)
        self.bin = bin
        self.bindir, tmp = os.path.split(bin)
        self.debug = False
        if self.bindir == "":
            self.bindir = "."
        self.binflags = binflags
        self.exit_on_error = False
        self.error_schedule_dir = \
                os.path.join(self.result_dir, "chess_schedules")
        self.addrlist = addrlist
        self.search_stack = searchstack.ChessSearchStack(0, self.addrlist)
        self.errors_found = {}
        self.preemption_bound = int(preemption_bound)

        print "Chess Checker Initialized"
        print "testing:", self.bin
        print "params:", self.binflags
        print "addrlist", self.addrlist
        print "preemption bound:", self.preemption_bound
        print "results directory:", self.result_dir
        sys.stdout.flush()

    def debugMode(self):
        self.debug = True

    def exitOnError(self):
        self.exit_on_error = True

    def getInitialSchedule(self):
        assert len(self.search_stack) == 0
        empty_schedule = schedule.Schedule()
        empty_schedule.addrlist = self.addrlist
        return self.testSchedule(empty_schedule)

    def generateSearchStack(self, schedule):
        pre = self.preemption_bound
        stack = self.search_stack.generateSearchStack(pre, schedule)
        self.search_stack = stack

    def isSearchComplete(self):
        return self.search_stack.isSearchComplete()

    def generateNextSchedule(self):
        return self.search_stack.generateNextSchedule()

    def testSchedule(self, test_schedule):
        self.executions += 1
        cwd = os.getcwd()
        os.chdir(self.bindir)
        output_file = os.path.join(self.bindir, self.thrille_in)
        test_schedule.outputExactSchedule(output_file)
        os.environ["LD_PRELOAD"] = self.npfthriller
        call_list = []
        call_list = [os.path.join(self.bindir, self.bin)] + self.binflags
        fout = open(os.path.join(self.bindir, "execution.log"), "w")
        exit = subprocess.call(call_list, stdout=fout, stderr=fout)
        #print call_list
        #exit = subprocess.call(call_list)
        fout.close()
        del os.environ["LD_PRELOAD"]
        os.remove(os.path.join(self.bindir, "execution.log"))
        os.chdir(cwd)
        mysched = \
                schedule.Schedule(os.path.join(self.bindir, self.thrille_out))
        return mysched

    def outputErrorSchedule(self, schedule):
        if not os.path.exists(self.error_schedule_dir):
            os.mkdir(self.error_schedule_dir)
        name = str(self.executions) + ".chess.err.schedule"
        name = os.path.join(self.error_schedule_dir, name)
        schedule.outputExactSchedule(name)

    def recordAnyErrors(self, schedule):
        err = schedule.getError()
        if not (err in self.errors_found) and not (err is None):
            print "Error (", err, ")",
            print "found in", self.executions, "executions"
            sys.stdout.flush()
            self.errors_found[err] = self.executions
            self.outputErrorSchedule(schedule)
            if self.exit_on_error:
                self.check_end_time = time.time()
                self.printResults()
                self.printResultsToFile()
                sys.exit()

    def regenerateSearchStack(self, target_sched, result_sched):
        self.search_stack.regenerate(target_sched, result_sched)

    def printDebugInfo(self, out):
        save = sys.stdout
        sys.stdout = out
        print "Executions: ", self.executions
        if len(self.errors_found) > 0:
            print "Errors:"
            for err in self.errors_found:
                print "\t", err,
                print "in",  self.errors_found[err], "iterations."
        else:
            print "No Errors Found"

        print self.search_stack.getDebugRepresentation(15)
        raw_input()
        sys.stdout = save


    def check(self):
        self.checkWithBound(999999999999)

    def checkWithBound(self, execution_bound):
        self.check_start_time = time.time()
        curr_sched = self.getInitialSchedule()
        self.generateSearchStack(curr_sched)
        while not self.isSearchComplete():
            if self.executions % 100 == 0:
                print "Number of Executions:", self.executions
                sys.stdout.flush()
            if self.debug:
                self.printDebugInfo(sys.__stdout__)
            if self.executions > execution_bound:
                self.completed_search = False
                break
            curr_sched = self.generateNextSchedule()
            if curr_sched is None:
                self.completed_search = True
                break
            result_sched = self.testSchedule(curr_sched)
            assert result_sched.getPreemptions() <= self.maxPreemptions()
            self.recordAnyErrors(result_sched)
            self.regenerateSearchStack(curr_sched, result_sched)
        self.check_end_time = time.time()

    def maxPreemptions(self):
        return self.preemption_bound

    def printResults(self):
        self.outputResults(sys.__stdout__)

    def printResultsToFile(self):
        fout = open(os.path.join(self.result_dir, "chess.results"), "w")
        self.outputResults(fout)
        fout.close()


    def outputResults(self, out):
        save = sys.stdout
        sys.stdout = out
        print
        if not self.completed_search:
            print
            print "Chess check with preemption bound",
            print self.preemption_bound, "was *NOT* completed, hit execution",
            print "bound at", self.executions, "executions"
        else:
            print 
            print "Chess check with preemption bound",
            print self.preemption_bound, "was completed in",
            print self.executions, "executions"
        print
        print "Chess Checking Results:"
        print "\tTotal Time:", self.check_end_time - self.check_start_time, 
        print "secs"
        print "\tFound Errors:"
        for err in self.errors_found:
            print "\t\t", err,
            print "in",  self.errors_found[err], "iterations."
        if len(self.errors_found) == 0:
            print "\tNo Found Errors"
        sys.stdout = save

def testSetup(test):
    tr = os.environ.get('THRILLE_ROOT')
    td = os.path.join(tr, "tests", "fwdrev", "chess")
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
    try:
        os.remove(os.path.join(td, "my-schedule"))
    except OSError:
        pass
    try:
        os.remove(os.path.join(td, "thrille-sched"))
    except OSError:
        pass

def test3():
    tr, td, bin = testSetup("03")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, "0", [], obj, bin, [])
    assert my_chesscheck.preemption_bound == 0
    my_chesscheck.check()

    assert my_chesscheck.executions == 11
    assert my_chesscheck.errors_found["segfault thread 0"] == 2


    testTeardown("03", tr, td, bin)
    return 1


def test4():
    tr, td, bin = testSetup("04")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 1
    assert len(my_chesscheck.errors_found) == 0


    my_chesscheck = ChessChecker(tr, 1, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 2
    assert my_chesscheck.errors_found["segfault thread 0"] == 2

    my_chesscheck = ChessChecker(tr, 2, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 4
    assert my_chesscheck.errors_found["segfault thread 0"] == 2

    testTeardown("04", tr, td, bin)
    return 1

def test5():
    tr, td, bin = testSetup("05")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 3
    assert len(my_chesscheck.errors_found) == 0

    my_chesscheck = ChessChecker(tr, 1, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.errors_found["segfault thread 0"] != 0


    testTeardown("05", tr, td, bin)
    return 1


def test6():
    tr, td, bin = testSetup("06")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 2
    assert len(my_chesscheck.errors_found) == 0

    testTeardown("06", tr, td, bin)
    return 1


def test7():
    tr, td, bin = testSetup("07")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 23
    assert len(my_chesscheck.errors_found) == 0

    testTeardown("07", tr, td, bin)
    return 1

def test8():
    tr, td, bin = testSetup("08")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 2
    assert len(my_chesscheck.errors_found) == 0

    testTeardown("08", tr, td, bin)
    return 1

def test9():
    tr, td, bin = testSetup("08")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, ["0x100", "0x200"], obj, bin, [])
    assert my_chesscheck.addrlist == ["0x100", "0x200"]
    
    first_sched = my_chesscheck.getInitialSchedule()
    assert first_sched.addrlist == ["0x100", "0x200"]

    testTeardown("08", tr, td, bin)
    return 1

def test10():
    tr, td, bin = testSetup("07")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.checkWithBound(14)
    assert my_chesscheck.executions == 15

    testTeardown("07", tr, td, bin)
    return 1

def test11():
    tr, td, bin = testSetup("09")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 0, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 18
    assert len(my_chesscheck.errors_found) == 1

    testTeardown("09", tr, td, bin)
    return 1

def fairschedTest():
    tr, td, bin = testSetup("10")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 1, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 19

    testTeardown("10", tr, td, bin)
    return 1

def fairschedTest2():
    tr, td, bin = testSetup("11")
    obj = os.path.join(tr, "obj")

    my_chesscheck = ChessChecker(tr, 100, [], obj, bin, [])
    my_chesscheck.check()
    assert my_chesscheck.executions == 174

    testTeardown("11", tr, td, bin)
    return 1


def test():
    test_passed = 0
    print "chess.py:"
    tr = os.environ.get('THRILLE_ROOT')
    td = os.path.join(tr, "tests", "fwdrev", "chess")
    assert os.path.exists(td)
    test_log = os.path.join(td, "simpltest.log")
    fout = open(test_log, "w")
    sys.stdout  = fout

    test_passed += test3()
    test_passed += test4()
    test_passed += test5()
    test_passed += test6()
    test_passed += test7()
    test_passed += test8()
    test_passed += test9()
    test_passed += test10()
    test_passed += test11()
    test_passed += fairschedTest()
    test_passed += fairschedTest2()
    
    sys.stdout = sys.__stdout__
    fout.close()
    os.remove(test_log)
    print "Tests Passed:", test_passed

def checkEnvironment():
    assert os.environ.get('THRILLE_ROOT') != None
    
    if len(sys.argv) < 5:
        if len(sys.argv) == 2 and sys.argv[1] == "-test":
            return

        print "usage: python chess.py  [preemption_bound]",
        print "[addr list] [result dir] [binary under test] [binary flags]"
        print
        print "testing: python pychess.py -test"
        print
        print "purpose: performs context bounded modelchecking",
        print "on [binary]. addrlist can be one of: 1)\"all\" 2)\"none\"",
        print "3) a path to a schedule with an addrlist in it"
        print
        sys.exit(1)

    try:
        x = int(sys.argv[1])
        assert x >= 0, "Preemption bound must be >= 0"
    except ValueError:
        assert False, "Preemption bound must be an integer"
    assert os.path.exists(sys.argv[4]), "Binary does not exist"
    if "all" not in sys.argv[2] and "none" not in sys.argv[2]:
        assert os.path.exists(sys.argv[2]), "addrlist does not exist"
    assert os.path.exists(sys.argv[3]), "results directory does not exist"

def main():
    checkEnvironment()
    tr = os.environ.get('THRILLE_ROOT')
    bound = sys.argv[1]
    if bound == "-test":
        test()
        return
    addrlist = []
    if "all" in sys.argv[2]: 
        addrlist = ["all"]
    elif "none" in sys.argv[2]:
        addrlist = []
    else:
        fin = open(sys.argv[2], 'r').readlines()
        assert "begin_addr_list" in fin.pop(0)
        while len(fin) > 0:
            item = fin.pop(0)
            if "end_addr_list" in item:
                break
            addrlist.append(item.strip())

    result_dir = os.path.abspath(sys.argv[3])
    bin = os.path.abspath(sys.argv[4])
    binflags = sys.argv[5:]
    new_binflags = []
    for x in binflags:
        if os.path.exists(x):
            x = os.path.abspath(x)
        new_binflags.append(x)
    my_chesscheck = \
            ChessChecker(tr, bound, addrlist, result_dir, bin, new_binflags)
    #my_chesscheck.debugMode() 
    my_chesscheck.exitOnError()
    my_chesscheck.checkWithBound(10000)
    my_chesscheck.printResults()
    my_chesscheck.printResultsToFile()
    
if __name__ == "__main__":
    main()
