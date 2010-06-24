import os
import sys
import copy
import subprocess
import shutil
import searchstack
import chess
import pickle
import time 

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule


class ChristosChessChecker(chess.ChessChecker):
    def __init__(self, tr, preemption_bound, addrlist, results, \
            initial_schedule, bin, binflags):
        chess.ChessChecker.__init__(self, tr, preemption_bound, addrlist, \
                results, bin, binflags)
        self.search_stack = \
                searchstack.ChessCheckpointedSearchStack(0, self.addrlist)
        assert os.path.exists(initial_schedule)
        self.initial_schedule = schedule.Schedule(initial_schedule)
    
    def getInitialSchedule(self):
        assert len(self.search_stack) == 0
        result_schedule = self.testSchedule(self.initial_schedule)
        assert result_schedule.getNumberOfCheckpoints() == 1
        checkpoint_prefix = result_schedule.getPrefixBeforeCheckpoint()
        assert checkpoint_prefix.getNumberOfCheckpoints() == 1
        self.preemptions_in_prefix = checkpoint_prefix.getPreemptions()
        print "preemptions in prefix:", self.preemptions_in_prefix
        print "max preemptions in a schedule:", self.maxPreemptions()
        print
        return self.testSchedule(checkpoint_prefix)

    def generateSearchStack(self, schedule):
        chess.ChessChecker.generateSearchStack(self, schedule)
        assert self.search_stack.getNumberOfCheckpoints() == 1

    def generateNextSchedule(self):
        next_sched = chess.ChessChecker.generateNextSchedule(self)
        if next_sched is not None:
            assert next_sched.getPreemptions() <= self.maxPreemptions()
        return next_sched

    def maxPreemptions(self):
        return self.preemption_bound + self.preemptions_in_prefix
    
    def isSearchComplete(self):
        complete = chess.ChessChecker.isSearchComplete(self)
        if complete:
            assert self.search_stack.getNumberOfCheckpoints() == 0
        else:
            assert self.search_stack.getNumberOfCheckpoints() == 1
    
    def regenerateSearchStack(self, target, result):
        chess.ChessChecker.regenerateSearchStack(self, target, result)
        number_preempts = self.search_stack.calculatePreemptionsInRange(0)
        assert number_preempts <= self.maxPreemptions()


def checkEnvironment():
    assert os.environ.get('THRILLE_ROOT') != None
    
    if len(sys.argv) < 6:
        if len(sys.argv) == 2 and sys.argv[1] == "-test":
            return

        print "usage: python christos_chess.py  [preemption_bound]",
        print "[addr list] [result dir] [intial schedule]",
        print "[binary under test] [binary flags]"
        print
        print "testing: python christos_chess.py -test"
        print
        print "purpose: CHESS checking with \"checkpoints\""
        print
        sys.exit(1)

    try:
        x = int(sys.argv[1])
        assert x >= 0, "Preemption bound must be >= 0"
    except ValueError:
        assert False, "Preemption bound must be an integer"
    assert os.path.exists(sys.argv[5]), "Binary does not exist"
    if "all" not in sys.argv[2] and "none" not in sys.argv[2]:
        assert os.path.exists(sys.argv[2]), "addrlist does not exist"
    assert os.path.exists(sys.argv[3]), "results directory does not exist"
    assert os.path.exists(sys.argv[4])

def test():
    print "TODO: implement tests for Christos Chess Checker"

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
    initial_schedule = os.path.abspath(sys.argv[4])
    bin = os.path.abspath(sys.argv[5])
    binflags = sys.argv[6:]
    new_binflags = []
    for x in binflags:
        if os.path.exists(x):
            x = os.path.abspath(x)
        new_binflags.append(x)
    my_chesscheck = \
            ChristosChessChecker(tr, bound, addrlist, result_dir,\
            initial_schedule, bin, new_binflags)
    #my_chesscheck.debugMode() 
    #my_chesscheck.exitOnError()
    my_chesscheck.check()
    my_chesscheck.printResults()
    my_chesscheck.printResultsToFile()
    
if __name__ == "__main__":
    main()

