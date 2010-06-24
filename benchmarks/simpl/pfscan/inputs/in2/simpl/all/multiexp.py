#this python file runs the non-iterative and iterative versions of 
#the A and B versions of the trace simplification algorithm,
#where:
#  A - Blocks are considered alone
#  B - Blocks of like thread segments are condensed into single blocks
#      this aims to simplify even in the case of incremental failure

import sys
import os
import copy
import noniterativeAA
import noniterativeBB
import shutil
tr = os.environ.get('THRILLE_ROOT')
if tr != None:
    sys.path.append(os.path.join(tr, "scripts/simpl/blockremoval"))
    import blockremoval
    sys.path.append(os.path.join(tr, "scripts/simpl/preemptremoval"))
    import preemptremoval

thrille_input_schedule = "thrille-sched"
thrille_output_schedule = "my-schedule"


def checkEnvironment():
    if len(sys.argv) < 4:
        print "usage: python multiexp.py [times to execute] [results dir]",
        print "[binary under test] [binary flags]"
        print
        print "purpose: performs the block removal AND",
        print "preemption simplification algorithm",
        print "on a generated input schedule in both their iterative",
        print "and non iterative forms, the specified number of times",
        print "saving the results to [results dir]"
        print
        print "intermediary/output files:"
        print "\tsimple-sched-NAA: output of noniterative AA algorithms"
        print "\tsimple-sched-NBB: output of noniterative BB algorithms"
        print "\tsimple-sched-IAA: output of iterative AA algorithms"
        print "\tsimple-sched-IBB: output of iterative BB algorithms"
        print
        print "definitions:\n\tA algorithm - each scheduling point is treated",
        print "as its own block, regardless of the blocks around it"
        print "\tB algorithm - all scheduling points choosing the same thread",
        print "in a row are coalesced and treated as one block"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert int(sys.argv[1]) > 0, "Nonsensical execution time"
    assert os.path.exists(sys.argv[2]), "Results directory does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"


def clearOldThrilleSchedule():
    open(thrille_input_schedule, "w").write("begin_addr_list\nend_addr_list\n")


def outputScheduleInformation(str, sched, enabled, condlist):
    blocks = blockremoval.getTotalBlocks(sched)
    contexts = preemptremoval.countContextSwitches(sched, enabled, condlist)
    npcs = preemptremoval.countNonpreemptiveCS(sched, enabled, condlist)
    preemptions = preemptremoval.countPreemptions(sched, enabled, condlist)
    assert (npcs + preemptions) == contexts, "sanity fail"
    print str, ":"
    print "\tblocks:", blocks, 
    print "\n\tcontext switches:", contexts, "\n\t\tpreemptions:",
    print preemptions, "\n\t\tnon-preemptive switches:", npcs

def executePreload(thrille_root, preload, binary, binflags):
    os.environ['LD_PRELOAD'] =  \
            os.path.join(thrille_root, "bin", preload)
    binarydir, bin = os.path.split(binary)
    thing = os.spawnve(os.P_NOWAIT, binary, binflags, os.environ)
    pid, exit = os.waitpid(thing, 0)
    del os.environ['LD_PRELOAD']
    return exit

def doRaceDetect(binary_file, binflags, thrille_root):
    clearOldThrilleSchedule()
    executePreload(thrille_root, "liblockrace.so", binary_file, binflags)

def getNewErrorSchedule(binary_file, thrille_root):
    assert os.path.exists(os.path.join(thrille_root, "bin/liblockrace.so"))
    assert os.path.exists(os.path.join(thrille_root, "bin/librandact.so"))
    binarydir, bin = os.path.split(binary_file)
    curr_dir = os.getcwd()
    if binarydir != '':
        os.chdir(binarydir)
    binflags = blockremoval.getBinaryFlags()
    binflags.insert(0, bin)
    print "NOTE: automatic race detection is disabled"
    #doRaceDetect(binary_file, binflags, thrille_root)
    assert os.path.exists("./thrille-randomactive")
    exit_status = 0
    sched = []
    enabled = []
    error = ""
    count = 0;
    while True:
        clearOldThrilleSchedule()
        count += 1
        if count > 1000:
            raw_input("100 iterations with no error--continue?")
            count = 0
        exit_status = executePreload(thrille_root, "librandact.so", \
                binary_file, binflags)
        print "Thrille Random Active Exit Status:", exit_status
        sched, enabled, addrlist, condlist = \
                blockremoval.readInNewSchedule(thrille_output_schedule)
        error = blockremoval.recordFailure(thrille_output_schedule)
        if error != None:
            if blockremoval.testSchedule(sched, error, addrlist,\
                    binary_file, thrille_root):
                os.chdir(curr_dir)
                return sched, enabled, addrlist, condlist, error
            else:
                assert False, "Error in Thrille makes replay impossible"

def runAlgoNAA(save, bin, thrille, sched, en, addrlist, err):
    #NAA 
    simpsched_NAA = noniterativeAA.noniterativeAA(save,\
            bin,\
            thrille, sched,\
            en, addrlist, err)

    blockremoval.outputResult(simpsched_NAA, err, addrlist,\
            bin, thrille, save)

    NAAsched, NAAenabled, NAAaddrlist, NAAcondlist = \
            blockremoval.readInNewSchedule(save)

    assert simpsched_NAA == NAAsched
    return NAAsched, NAAenabled, NAAaddrlist, NAAcondlist

def runAlgoNBB(save, bin, thrille, sched, en, addrlist, err):
    #NBB 
    simpsched_NBB = noniterativeBB.noniterativeBB(save,\
            bin,\
            thrille, sched,\
            en, addrlist, err)

    blockremoval.outputResult(simpsched_NBB, err, addrlist,\
            bin, thrille, save)

    NBBsched, NBBenabled, NBBaddrlist, NBBcondlist = \
            blockremoval.readInNewSchedule(save)

    assert simpsched_NBB == NBBsched
    return NBBsched, NBBenabled, NBBaddrlist, NBBcondlist

def runAlgoIAA(save, bin, thrille, sched, en, addrlist, err):
    prev_IAAsched = sched
    prev_IAAenabled = en
    IAAsched = []
    IAAenabled = []
    IAAaddrlist = []
    IAAcondlist = []
    while True:
        simpsched_IAA = noniterativeAA.noniterativeAA(save,\
                bin,\
                thrille, prev_IAAsched,\
                prev_IAAenabled, addrlist, err)

        blockremoval.outputResult(simpsched_IAA, err, addrlist,\
                bin, thrille, save)

        IAAsched, IAAenabled, IAAaddrlist, IAAcondlist = \
                blockremoval.readInNewSchedule(save)

        assert simpsched_IAA == IAAsched
        assert IAAaddrlist == addrlist

        if IAAsched == prev_IAAsched:
            break
        else:
            prev_IAAsched = IAAsched
            prev_IAAenabled = IAAenabled

    return IAAsched, IAAenabled, IAAaddrlist, IAAcondlist

def runAlgoIBB(save, bin, thrille, sched, en, addrlist, err):
    prev_IBBsched = sched
    prev_IBBenabled = en
    IBBsched = []
    IBBenabled = []
    IBBaddrlist = []
    IBBcondlist = []
    while True:
        simpsched_IBB = noniterativeBB.noniterativeBB(save,\
                bin,\
                thrille, prev_IBBsched,\
                prev_IBBenabled, addrlist, err)

        blockremoval.outputResult(simpsched_IBB, err, addrlist,\
                bin, thrille, save)

        IBBsched, IBBenabled, IBBaddrlist, IBBcondlist = \
                blockremoval.readInNewSchedule(save)

        assert simpsched_IBB == IBBsched
        assert IBBaddrlist == addrlist

        if IBBsched == prev_IBBsched:
            break
        else:
            prev_IBBsched = IBBsched
            prev_IBBenabled = IBBenabled

    return IBBsched, IBBenabled, IBBaddrlist, IBBcondlist

def main():
    checkEnvironment()
    times_to_repeat = int(sys.argv[1])
    save_directory = sys.argv[2]
    binary_file = sys.argv[3]
    thrille_root = os.environ.get('THRILLE_ROOT')
    fout = open(os.path.join(save_directory, "simpl-runstat"), "w")
    my_bin_save = os.path.join(save_directory, "bin")
    os.mkdir(my_bin_save)
    shutil.copy(os.path.join(thrille_root, "bin", "libserializer.so"), \
            os.path.join(my_bin_save, "libserializer.so"))
    shutil.copy(os.path.join(thrille_root, "bin", "libstrictserial.so"), \
            os.path.join(my_bin_save, "libstrictserial.so"))
    shutil.copy(os.path.join(thrille_root, "bin", "librandomschedule.so"), \
            os.path.join(my_bin_save, "librandomschedule.so"))
    shutil.copy(os.path.join(thrille_root, "bin", "librandact.so"), \
            os.path.join(my_bin_save, "librandact.so"))
    shutil.copy(os.path.join(thrille_root, "bin", "librace.so"), \
            os.path.join(my_bin_save, "librace.so"))
    shutil.copy(os.path.join(thrille_root, "bin", "liblockrace.so"), \
            os.path.join(my_bin_save, "liblockrace.so"))

    #figure out how to remove svn
    os.mkdir(os.path.join(save_directory, "src"))

    shutil.copytree(os.path.join(thrille_root, "src"), \
            os.path.join(save_directory,"src","src")) \

    shutil.copytree(os.path.join(thrille_root, "scripts"), \
            os.path.join(save_directory,"src","scripts"))
    fout.write("Command that was run:\n")
    for x in sys.argv:
        fout.write(x + " ")
    fout.write("\n\n")

    #lists for tracking statistics
    start_list = []
    naa_list = []
    nbb_list = []
    iaa_list = []
    ibb_list = [] 
    i = 0
    while i < times_to_repeat:
        my_save_dir = ""
        if (i < 10):
            my_save_dir = os.path.join(save_directory, "run0" + str(i))
        else:
            my_save_dir = os.path.join(save_directory, "run" + str(i))
        os.mkdir(my_save_dir)

        startsched, startenabled, startaddrlist, startcondlist, error = \
                getNewErrorSchedule(binary_file, thrille_root)


        #save the error schedule we are starting with
        #and ensure it's legitimate
        blockremoval.outputResult(startsched, error, startaddrlist, \
                binary_file, thrille_root, \
                os.path.join(my_save_dir, "start-sched"))

        start_list.append((startsched, startenabled, startcondlist))
        
        #NAA
        output_schedule = os.path.join(my_save_dir, "simp-sched-NAA")
        NAAsched, NAAenabled, NAAaddrlist, NAAcondlist = \
                runAlgoNAA(output_schedule, binary_file, thrille_root, \
                startsched, startenabled, startaddrlist, error)
        assert NAAaddrlist == startaddrlist
        naa_list.append((NAAsched, NAAenabled, NAAcondlist))

        #NBB
        output_schedule = os.path.join(my_save_dir, "simp-sched-NBB")
        NBBsched, NBBenabled, NBBaddrlist, NBBcondlist = \
                runAlgoNBB(output_schedule, binary_file, thrille_root, \
                startsched, startenabled, startaddrlist, error)
        assert NBBaddrlist == startaddrlist
        nbb_list.append((NBBsched, NBBenabled, NBBcondlist))

        #IAA - iterate until fixed point
        output_schedule = os.path.join(my_save_dir, "simp-sched-IAA")
        IAAsched, IAAenabled, IAAaddrlist, IAAcondlist = \
                runAlgoIAA(output_schedule, binary_file, thrille_root, \
                startsched, startenabled, startaddrlist, error)
        assert IAAaddrlist == startaddrlist
        assert len(IAAsched) <= len(NAAsched)
        iaa_list.append((IAAsched, IAAenabled, IAAcondlist))

        #IBB - iterate until fixed point
        output_schedule = os.path.join(my_save_dir, "simp-sched-IBB")
        IBBsched, IBBenabled, IBBaddrlist, IBBcondlist = \
                runAlgoIBB(output_schedule, binary_file, thrille_root, \
                startsched, startenabled, startaddrlist, error)
        assert IBBaddrlist == startaddrlist
        assert len(IBBsched) <= len(NBBsched)
        ibb_list.append((IBBsched, IBBenabled, IBBcondlist))

        assert len(start_list) == len(naa_list)
        assert len(naa_list) == len(nbb_list)
        assert len(nbb_list) == len(iaa_list)
        assert len(iaa_list) == len(ibb_list)
        sched, en, cond = start_list[-1]
        outputScheduleInformation("Start", sched, en, cond) 
        sched, en, cond = naa_list[-1]
        outputScheduleInformation("NAA", sched, en, cond) 
        sched, en, cond = nbb_list[-1]
        outputScheduleInformation("NBB", sched, en, cond) 
        sched, en, cond = iaa_list[-1]
        outputScheduleInformation("IAA", sched, en, cond) 
        sched, en, cond = ibb_list[-1]
        outputScheduleInformation("IBB", sched, en, cond) 

        tmpout = open(os.path.join(my_save_dir, "README"), "w")
        sys.stdout = tmpout 
        sched, en, cond = start_list[-1]
        outputScheduleInformation("Start", sched, en, cond) 
        sched, en, cond = naa_list[-1]
        outputScheduleInformation("NAA", sched, en, cond) 
        sched, en, cond = nbb_list[-1]
        outputScheduleInformation("NBB", sched, en, cond) 
        sched, en, cond = iaa_list[-1]
        outputScheduleInformation("IAA", sched, en, cond) 
        sched, en, cond = ibb_list[-1]
        outputScheduleInformation("IBB", sched, en, cond) 
        tmpout.write("\n")
        sys.stdout.flush()
        sys.stdout = sys.__stdout__
        tmpout.close()


        fout.write("**RUN " + str(i) + "\n")
        sys.stdout = fout
        sched, en, cond = start_list[-1]
        outputScheduleInformation("Start", sched, en, cond) 
        sched, en, cond = naa_list[-1]
        outputScheduleInformation("NAA", sched, en, cond) 
        sched, en, cond = nbb_list[-1]
        outputScheduleInformation("NBB", sched, en, cond) 
        sched, en, cond = iaa_list[-1]
        outputScheduleInformation("IAA", sched, en, cond) 
        sched, en, cond = ibb_list[-1]
        outputScheduleInformation("IBB", sched, en, cond) 
        fout.write("\n")
        sys.stdout.flush()
        sys.stdout = sys.__stdout__
        i+= 1

    #output statistics  


if __name__ == "__main__":
    main()


