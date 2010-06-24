# The Tineratia algorithm 
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
import pickle
import time 

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule

class Simplifier:
    def __init__(self, tr, relaxedserial, strictserial, input, bin, binflags):
        self.thrille_out = "my-schedule"
        self.thrille_in = "thrille-relaxed-sched"
        self.transform_stat_blocks = {}
        self.transform_stat_threads = {}
        self.transform_stat_ctxt = {}
        self.transform_stat_nonpre = {}
        self.transform_stat_pre = {}
        self.relaxedserial = relaxedserial
        self.strictserial = strictserial
        self.empty_schedules = 0
        self.our_bug_count = 0
        self.other_bug_count = 0
        self.no_bug_count = 0
        self.time_fails = 0
        self.executions = 0
        self.iterations = 0
        self.tr = tr
        self.input = input
        self.bin = bin
        self.bindir, tmp = os.path.split(bin)
        if self.bindir == "":
            self.bindir = "."
        self.binflags = binflags
        self.start_schedule = schedule.Schedule(input)
        if self.start_schedule.error is not None:
            assert "THRILLE INTERNAL ERROR" not in self.start_schedule.error
            assert "unknown error" not in self.start_schedule.error
        assert os.path.exists(self.relaxedserial)
        sanitycheck = self.testScheduleStrict(self.start_schedule).error
        print "Sanity check error:", sanitycheck
        print "Expected error:", self.start_schedule.error
        assert  sanitycheck == self.start_schedule.error
        dfail = os.path.join(self.tr, "bin", "libdfailserial.so")
        assert os.path.exists(dfail)
        self.dfail = dfail


    def testScheduleWithThriller(self, sched, thriller):
        cwd = os.getcwd()
        os.chdir(self.bindir)
        assert os.path.exists(thriller)
        output_schedule = os.path.join(self.bindir, self.thrille_in)
        sched.outputRelaxedSchedule(output_schedule)
        #sched.outputExactSchedule(output_schedule)
        #print "len:",sched.getScheduleLength()
        #raw_input()
        os.environ["LD_PRELOAD"] = thriller
        call_list = []
        call_list = [os.path.join(self.bindir, self.bin)] + self.binflags
        fout = open(os.path.join(self.bindir, "execution.log"), "w")
        exit = subprocess.call(call_list, stdout=fout, stderr=fout)
        #exit = subprocess.call(call_list)
        #raw_input()
        fout.close()
        del os.environ["LD_PRELOAD"]
        os.remove(os.path.join(self.bindir, "execution.log"))
        os.chdir(cwd)
        mysched = schedule.Schedule(os.path.join(self.bindir, self.thrille_out))
        return mysched

    def testScheduleRelaxed(self, sched):
        if sched == schedule.Schedule():
            self.empty_schedules += 1
            return sched
        
        # timeouts
        time_start = time.time()
        mysched = self.testScheduleWithThriller(sched, self.relaxedserial)
        time_end = time.time()
        time_diff = time_end - time_start
        if time_diff > 35:
            self.time_fails += 1
            return schedule.Schedule()

        if mysched.error is None:
            self.no_bug_count += 1
        else:
            if mysched.error == self.start_schedule.error:
                self.our_bug_count += 1
            else:
                self.other_bug_count += 1
        self.executions += 1
        return mysched

    # just used for (optional) sanity check, don't keep statistics on these 
    # executions
    def testScheduleStrict(self, sched):
        if sched == schedule.Schedule():
            self.empty_schedules += 1
            return sched
        return self.testScheduleWithThriller(sched, self.strictserial)

    def testScheduleDfail(self, sched):
        if sched == schedule.Schedule():
            self.empty_schedules += 1
            return sched

        return self.testScheduleWithThriller(sched, self.dfail)


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
        assert self.testScheduleStrict(sched).error == sched.error
        threadOrder = sched.getThreadOrder()
        threadOrder.reverse()
        thread_done = {}
        return_sched = sched.cloneThyself()
        for x in threadOrder:
            if x not in thread_done:
                untested_sched = return_sched.removeFinalBlockOfThread(x)

               
                return_ctxt = return_sched.getContextSwitches()
                untested_ctxt = untested_sched.getContextSwitches()
                assert return_ctxt >= untested_ctxt

                tested_sched = self.testScheduleRelaxed(untested_sched)
                if tested_sched.error == return_sched.error:
                    #termination condition
                    return_ctxt = return_sched.getContextSwitches()
                    tested_ctxt = tested_sched.getContextSwitches()
                    if tested_ctxt <= return_ctxt:
                        return_sched = tested_sched
                else:
                    thread_done[x] = True
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def failedInMovedTEI(self, tested_sched, i):
        if tested_sched == schedule.Schedule():
            return False
        return tested_sched.isInLastTEI(i)

    def synthesizeSchedules(self, result, original, tei):
        return result.synthesizeForwardSchedule(original, tei)

    # This function will attempt to move a whole TEI up.  We test the new
    # schedule with a special thriller (Dfail) which implements the relaxations
    # of addrserial, but will fail out if a thread becomes disabled during a
    # TEI.  If we fail in the TEI which was moved up, we move the prefix of the
    # TEI which was executable up and leave the rest in the original position.
    # We then test this again with the standard (RTEST) relaxed feasibility
    # tester.
    def twoStageConsolidation(self, sched, i):
        untested_sched, tei = sched.consolidateFrontierForwardTEI(i)

        # test schedule to see if we can move the whole TEI
        #tested_sched = self.testScheduleDfail(untested_sched)
        tested_sched = self.testScheduleStrict(untested_sched)
        
        if self.failedInMovedTEI(tested_sched, i):
            # Thread became disabled in the moved TEI,
            # move as much of the TEI up as possible
            # and leave the rest down
            untested_sched = self.synthesizeSchedules(tested_sched, sched, tei)
           
        # still test the overall thing in a relaxed fashion
        return self.testScheduleRelaxed(untested_sched)


    def simplifyForwardTwoStage(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():

            tested_sched = self.twoStageConsolidation(return_sched, i)

            if tested_sched.error == return_sched.error:
                #termination condition
                return_ctxt = return_sched.getContextSwitches()
                tested_ctxt = tested_sched.getContextSwitches()
                if tested_ctxt <= return_ctxt:
                    return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched




    def simplifyForward(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.consolidateFrontierForward(i)

            return_ctxt = return_sched.getContextSwitches()
            untested_ctxt = untested_sched.getContextSwitches()
            assert return_ctxt >= untested_ctxt

            tested_sched = self.testScheduleRelaxed(untested_sched)
            if tested_sched.error == return_sched.error:
                #termination condition
                tested_ctxt = tested_sched.getContextSwitches()
                if tested_ctxt < return_ctxt:
                    return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def simplifyBackward(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = sched.getScheduleLength() - 1
        return_sched = sched.cloneThyself()
        #print "START", return_sched
        #raw_input()
        assert return_sched.getScheduleLength() == (i + 1)
        while i > 0:
            # check if some change we made shortened the schedule
            # i.e. we hit the error earlier
            if i >= return_sched.getScheduleLength():
                i = (return_sched.getScheduleLength() - 1)
            untested_sched = return_sched.consolidateFrontierBackward(i)

            return_ctxt = return_sched.getContextSwitches()
            untested_ctxt = untested_sched.getContextSwitches()
            assert return_ctxt >= untested_ctxt
            #print "original", sched
            #print "untest", untested_sched 

            tested_sched = self.testScheduleRelaxed(untested_sched)
            #print "test", tested_sched 
            #print "i:",i

            #raw_input()
            if tested_sched.error == return_sched.error:
                #termination condition
                return_ctxt = return_sched.getContextSwitches()
                tested_ctxt = tested_sched.getContextSwitches()
                if tested_ctxt <= return_ctxt:
                    return_sched = tested_sched
            i -= 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def blockExtend(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.blockExtend(i)
            tested_sched = self.testScheduleRelaxed(untested_sched)
            if tested_sched.error == return_sched.error:
                return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def blockExtendEBO(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.blockExtendEqualButOpposite(i)
            tested_sched = self.testScheduleRelaxed(untested_sched)
            if tested_sched.error == return_sched.error:
                return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def preemptExtendLastTEI(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        return_sched = sched.cloneThyself()
        i = return_sched.getScheduleLength() - 1 
        done_list = []
        while i >= 0:
            if return_sched.schedule[i].caller not in done_list:
                print return_sched.schedule[i]
                done_list.append(return_sched.schedule[i].caller)
                untested_sched = return_sched.preemptExtendSimple(i)
                print untested_sched
                tested_sched = self.testScheduleRelaxed(untested_sched)
                if tested_sched.error == return_sched.error:
                    return_pre = return_sched.getPreemptions()
                    tested_pre = tested_sched.getPreemptions()
                    if tested_pre < return_pre:
                        return_sched = tested_sched
            i -= 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    
    def preemptExtendSimple(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.preemptExtendSimple(i)
            tested_sched = self.testScheduleRelaxed(untested_sched)
            if tested_sched.error == return_sched.error:
                return_pre = return_sched.getPreemptions()
                tested_pre = tested_sched.getPreemptions()
                if tested_pre < return_pre:

                #return_blocks = len(return_sched.getThreadOrder())
                #tested_blocks = len(tested_sched.getThreadOrder())
                #if tested_blocks < return_blocks:
                    return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def preemptExtend(self, sched):
        assert self.testScheduleStrict(sched).error == sched.error
        i = 0
        return_sched = sched.cloneThyself()
        while i < return_sched.getScheduleLength():
            untested_sched = return_sched.preemptExtend(i)
            tested_sched = self.testScheduleRelaxed(untested_sched)
            if tested_sched.error == return_sched.error:
                tpre = tested_sched.getPreemptions()
                rpre = return_sched.getPreemptions()
                if tpre < rpre:
                    return_sched = tested_sched
            i += 1
        assert self.testScheduleStrict(return_sched).error == sched.error
        return return_sched

    def simplifyForwardFixpoint(self, sched, debug):
        j = 0
        for_last = schedule.Schedule()
        for_current = sched 
        while for_last != for_current:
            for_last = for_current
            for_current = self.simplifyForward(for_current)
            for_current = self.preemptExtendSimple(for_current)
            j += 1
        if debug:
            self.printEffect("Forward Consolidation", \
                    sched , for_current)
            print "\t\tIterations until forward simplification",
            print "hit a fix point:", j
        return for_current

    def simplifyBackwardFixpoint(self, sched, debug):
        j = 0
        back_last = schedule.Schedule()
        back_current = sched
        while back_last != back_current:
            back_last = back_current
            back_current = self.simplifyBackward(back_current)
            back_current = self.preemptExtendSimple(back_current)
            j += 1
        if debug:
            self.printEffect("Backward Consolidation", \
                    sched, back_current)
            print "\t\tIterations until backward simplification",
            print "hit a fix point:", j
        return back_current

    def blockRemoveDebug(self, sched, debug):
        br_sched = self.blockRemove(sched)
        if debug:
            self.printEffect("Block Removal", sched, br_sched)
        return br_sched

    def blockExtendDebug(self, sched, debug):
        be_sched = self.blockExtend(sched)
        if debug:
            self.printEffect("Block Extend", sched, be_sched)
        return be_sched

    def blockExtendEBODebug(self, sched, debug):
        be_sched = self.blockExtendEBO(sched)
        if debug:
            self.printEffect("Block Extend Equal but Opposite", \
                    sched, be_sched)
        return be_sched

    def preemptExtendDebug(self, sched, debug):
        pext_sched = self.preemptExtend(sched)
        if debug:
            self.printEffect("Preemption Extender", sched, pext_sched)
        return pext_sched


    def printEffect(self, string, oldsched, newsched):
        print "\t", string, ":"
        oldblock = oldsched.getScheduleLength()
        newblock = newsched.getScheduleLength()
        print "\t\tSchedule Points Removed:", oldblock - newblock
        oldthr = oldsched.getNumberOfThreads()
        newthr = newsched.getNumberOfThreads()
        print "\t\tThreads Removed:", oldthr - newthr
        oldctxt = oldsched.getContextSwitches()
        newctxt = newsched.getContextSwitches()
        print "\t\tContext Switches Removed:", oldctxt - newctxt
        oldnpcs = oldsched.getNonPreemptiveContextSwitches()
        newnpcs = newsched.getNonPreemptiveContextSwitches()
        print "\t\tNon-Preemptive Context Switches Removed:", oldnpcs - newnpcs
        oldpre = oldsched.getPreemptions()
        newpre = newsched.getPreemptions()
        print "\t\tPreemptions Removed:", oldpre - newpre


        assert (oldnpcs + oldpre) == oldctxt
        assert (newnpcs + newpre) == newctxt

        if not self.transform_stat_blocks.has_key(string):
            self.transform_stat_blocks[string] = 0

        if not self.transform_stat_threads.has_key(string):
            self.transform_stat_threads[string] = 0

        if not self.transform_stat_ctxt.has_key(string):
            self.transform_stat_ctxt[string] = 0

        if not self.transform_stat_nonpre.has_key(string):
            self.transform_stat_nonpre[string] = 0

        if not self.transform_stat_pre.has_key(string):
            self.transform_stat_pre[string] = 0

        self.transform_stat_blocks[string] += (oldblock - newblock)
        self.transform_stat_threads[string] += (oldthr - newthr)
        self.transform_stat_ctxt[string] += (oldctxt - newctxt)
        self.transform_stat_nonpre[string] += (oldnpcs - newnpcs)
        self.transform_stat_pre[string] += (oldpre - newpre)

    def printScheduleInfo(self, sched):
        sched_str = sched.getSummaryInfo()
        print
        print "Schedule Statistics"
        print sched_str
        print

    def ensureSanity(self, sched):

        stat_removed_blocks = 0
        for x in self.transform_stat_blocks:
            stat_removed_blocks += self.transform_stat_blocks[x]
     
        
        stat_removed_threads = 0
        for x in self.transform_stat_threads:
            stat_removed_threads += self.transform_stat_threads[x]
      
        
        stat_removed_ctxt = 0
        for x in self.transform_stat_ctxt:
            stat_removed_ctxt += self.transform_stat_ctxt[x]
       
        
        stat_removed_npcs = 0
        for x in self.transform_stat_nonpre:
            stat_removed_npcs += self.transform_stat_nonpre[x]


        stat_removed_pre = 0
        for x in self.transform_stat_pre:
            stat_removed_pre += self.transform_stat_pre[x]

        assert (stat_removed_pre + stat_removed_npcs) == stat_removed_ctxt

        for x in self.transform_stat_ctxt:
            n = self.transform_stat_nonpre[x]
            p = self.transform_stat_pre[x]
            c = self.transform_stat_ctxt[x]
            assert (n + p) == c


        startblocks = self.start_schedule.getScheduleLength()
        endblocks = sched.getScheduleLength()
        real_blocks = startblocks - endblocks

        startthreads = self.start_schedule.getNumberOfThreads()
        endthreads = sched.getNumberOfThreads()
        real_threads = startthreads - endthreads
       
        startctxt = self.start_schedule.getContextSwitches()
        endctxt = sched.getContextSwitches()
        real_ctxt = startctxt - endctxt

        startnpcs = self.start_schedule.getNonPreemptiveContextSwitches()
        endnpcs = sched.getNonPreemptiveContextSwitches()
        real_npcs = startnpcs - endnpcs


        startpre = self.start_schedule.getPreemptions()
        endpre = sched.getPreemptions()
        real_pre = startpre - endpre

        assert real_blocks == stat_removed_blocks
        assert real_threads == stat_removed_threads
        assert real_ctxt == stat_removed_ctxt
        assert real_npcs == stat_removed_npcs
        assert real_pre == stat_removed_pre



    def simplify(self, debug=True):
        if self.start_schedule.error is None:
            return self.start_schedule

        last_sched = schedule.Schedule()
        current_sched = self.start_schedule.cloneThyself()
        i = 0
        while current_sched.getContextSwitches() != last_sched.getContextSwitches():
            if i == 0:
                self.printScheduleInfo(current_sched)
            tmp_sched = current_sched
            
            rtmp_sched = self.blockRemoveDebug(tmp_sched, debug)

            ftmp_sched = self.simplifyForwardTwoStage(rtmp_sched)
            self.printEffect("Forward Consolidation2", rtmp_sched, ftmp_sched)
            
            btmp_sched = self.simplifyBackward(ftmp_sched)
            self.printEffect("Backward Consolidation", ftmp_sched, btmp_sched)


            last_sched = current_sched
            current_sched = btmp_sched
            i += 1
        
        self.iterations = i

        assert current_sched.error == self.start_schedule.error
        tmp_error = self.testScheduleStrict(current_sched).error
        assert tmp_error == self.start_schedule.error
        self.ensureSanity(current_sched)
        return current_sched

    def getStartSched(self):
        return self.start_schedule



def checkEnvironment():
   
    if len(sys.argv) < 4:
        if len(sys.argv) == 2 and sys.argv[1] == "-test":
            assert os.environ.get('THRILLE_ROOT') != None
            return

        print "usage: python tinertia.py  [input schedule] [output schedule]",
        print "[binary under test] [binary flags]"
        print
        print "testing: python tinertia.py -test"
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
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "program deadlock" in s.start_schedule.error
    assert s.start_schedule.getNumberOfThreads() == 4
    news = s.blockRemove(s.start_schedule)
    assert news.getNumberOfThreads() == 4

    schedin = os.path.join(td, "01.sched2")
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert s.start_schedule.error is None
    end_sched = s.blockRemove(s.start_schedule) 
    assert end_sched.getNumberOfThreads() == 2
    assert end_sched.getScheduleLength() == 57
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "segfault" in s.start_schedule.error
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
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "program deadlock" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 11
    assert s.start_schedule.getPreemptions() == 8
    news = s.simplifyForward(s.start_schedule)
    print news.getContextSwitches()
    assert news.getContextSwitches() == 4
    assert news.getPreemptions() == 1
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "segfault" in s.start_schedule.error
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
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "program deadlock" in s.start_schedule.error
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
    relax = os.path.join(tr, "bin", "libaddrserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "segfault" in s.start_schedule.error
    assert s.start_schedule.getContextSwitches() == 8
    assert s.start_schedule.getPreemptions() == 6
    brs = s.blockRemove(s.start_schedule)
    news = s.simplifyBackward(brs)
    assert news.getContextSwitches() == 2
    assert news.getPreemptions() == 0


    news = s.simplifyBackward(s.start_schedule)
    assert news.getContextSwitches() == 3
    assert news.getPreemptions() == 1
    assert news.getNumberOfThreads() == 3
    testTeardown("02", tr, td, bin)
    return 1    

def testBlockExtend():
    tr, td, bin = testSetup("03")
    schedin = os.path.join(td, "03.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "segfault" in s.start_schedule.error
    assert s.start_schedule.getNumberOfThreads() == 4
    news = s.blockExtend(s.start_schedule)
    assert news.getNumberOfThreads() == 4
    testTeardown("03", tr, td, bin)
    
    tr, td, bin = testSetup("04")
    schedin = os.path.join(td, "04.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    assert "segfault" in s.start_schedule.error
    assert s.start_schedule.getNumberOfThreads() == 4
    assert s.start_schedule.getPreemptions() == 3
    news = s.blockExtend(s.start_schedule)
    assert news.getNumberOfThreads() == 4
    assert news.getPreemptions() == 0
    testTeardown("04", tr, td, bin)
    return 1

def testSimplify():
    tr, td, bin = testSetup("01")
    fout = open(td + "simplify.log", "w")
    sys.stdout = fout
    schedin = os.path.join(td, "01.sched1")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    result = s.simplify()
    assert result.getNumberOfThreads() == 4
    assert result.getContextSwitches() == 4
    assert result.getPreemptions() == 1
    testTeardown("01", tr, td, bin)

    tr, td, bin = testSetup("02")
    schedin = os.path.join(td, "02.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    result = s.simplify()
    assert result.getNumberOfThreads() == 2
    assert result.getContextSwitches() == 2
    assert result.getPreemptions() == 0
    testTeardown("02", tr, td, bin)
    
    tr, td, bin = testSetup("05")
    schedin = os.path.join(td, "05.sched")
    assert os.path.exists(schedin)
    binflags = []
    relax = os.path.join(tr, "bin", "librelaxedserial.so")
    tester = os.path.join(tr, "bin", "librelaxedtester.so")
    s = Simplifier(tr, relax, tester, schedin, bin, binflags)
    result = s.simplify()
    assert result.getPreemptions() == 0
    testTeardown("05", tr, td, bin)
    
    sys.stdout = sys.__stdout__
    fout.close()
    os.remove(td + "simplify.log")
    return 1


def test():
    test_passed = 0
    print "tinertia.py:"
    tr = os.environ.get('THRILLE_ROOT')
    td = os.path.join(tr, "tests", "fwdrev")
    assert os.path.exists(td)
    test_log = os.path.join(td, "simpltest.log")
    fout = open(test_log, "w")
    sys.stdout  = fout
    test_passed += testBlockRemove()
    test_passed += testSimplifyForward()
    test_passed += testSimplifyBackward()
    test_passed += testBlockExtend()
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

    relaxed = os.path.join(tr, "bin", "libaddrserial.so")
    strict_test = os.path.join(tr, "bin", "librelaxedtester.so")
    assert os.path.exists(relaxed)
    assert os.path.exists(strict_test)
    s = Simplifier(tr, relaxed, strict_test, input, bin, binflags)
    start_time = time.time()
    done_sched = s.simplify()
    end_time = time.time()
    simplify_time = end_time - start_time
    start_sched = s.getStartSched()
    start_str = start_sched.getSummaryInfo()
    done_str = done_sched.getSummaryInfo()
    print "Start Schedule:"
    print start_str
    print
    print "Simplified Schedule:"
    print done_str
    print
    print "Number of Iterations:", s.iterations
    print "Number of Executions:", s.executions
    print "Time (sec):", simplify_time
    print "Number of empty schedules:", s.empty_schedules
    print "Number of generated schedules which show our bug:", s.our_bug_count
    print "Number of generated schedules which show a different bug:",
    print s.other_bug_count
    print "Number of generated schedules which show no bug:", s.no_bug_count
    print
    done_sched.outputRelaxedSchedule(output)

if __name__ == "__main__":
    main()
