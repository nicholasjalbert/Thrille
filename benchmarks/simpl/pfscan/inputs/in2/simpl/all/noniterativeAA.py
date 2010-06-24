import sys
import os
import copy
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
        print "usage: python noniterative.py [input schedule]",
        print "[output schedule] [binary under test] [binary flags]"
        print
        print "purpose: performs the block removal AND",
        print "preemption simplification algorithm",
        print "on the input schedule and outputs the results"
        print
        print "intermediary files:\n\tblock-sched: the schedule after",
        print "block removal was performed"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert os.path.exists(sys.argv[1]), "input does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"
    assert "block-sched" not in sys.argv[1], "Input file will be overwritten"
    assert "my_schedule" not in sys.argv[1], "Input file will be overwritten"
    assert "thrille-sched" not in sys.argv[1], "Input file will be overwritten"
    assert sys.argv[1] != sys.argv[2], "Input file will be overwritten"
    assert "block-sched" not in sys.argv[2], "Output file error"
    assert "my_schedule" not in sys.argv[2], "Output file error"
    assert "thrille-sched" not in sys.argv[2], "Output file error"


def noniterativeAA(output_schedule, binary_file, thrille_root,\
        sched, enabled, addrlist, error):
    brsched = blockremoval.blockRemovalAlgorithmA( \
            binary_file, thrille_root, sched, enabled, addrlist, error)
    
    outdir, out = os.path.split(output_schedule)
    tmpout = os.path.join(outdir, "block-sched")

    blockremoval.outputResult(brsched, error, addrlist, binary_file,\
            thrille_root, tmpout) 
    newsched, newenabled, newaddrlist, newcondlist = \
            blockremoval.readInNewSchedule(tmpout)

    assert blockremoval.recordFailure(tmpout) == error, "err fail"

    simpsched = preemptremoval.preemptRemovalAlgorithmA(binary_file, \
            output_schedule, thrille_root, newsched, newenabled, \
            newaddrlist, newcondlist, error)

    return simpsched




def main():
    checkEnvironment()
    input_schedule = sys.argv[1]
    output_schedule = sys.argv[2]
    binary_file = sys.argv[3]
    thrille_root = os.environ.get('THRILLE_ROOT')
    sched, enabled, addrlist, condlist = \
            blockremoval.readInNewSchedule(sys.argv[1])
    error = blockremoval.recordFailure(sys.argv[1])

    assert blockremoval.testSchedule(sched, error, addrlist, binary_file, \
            thrille_root)

    start_blocks = blockremoval.getTotalBlocks(sched)
    start_context = \
            preemptremoval.countContextSwitches(sched, enabled, condlist)
    start_npcs = \
            preemptremoval.countNonpreemptiveCS(sched, enabled, condlist)
    start_preemptions = \
            preemptremoval.countPreemptions(sched, enabled, condlist)

    assert (start_npcs + start_preemptions) == start_context, "sanity fail"

    simpsched = noniterativeAA(output_schedule, binary_file,\
            thrille_root, sched,\
            enabled, addrlist, error)

    blockremoval.outputResult(simpsched, error, addrlist,\
            binary_file, thrille_root, output_schedule)

    sched, enabled, addrlist, condlist = \
            blockremoval.readInNewSchedule(output_schedule)
    end_blocks = blockremoval.getTotalBlocks(sched)
    end_context = \
            preemptremoval.countContextSwitches(sched, enabled, condlist)
    end_npcs = \
            preemptremoval.countNonpreemptiveCS(sched, enabled, condlist)
    end_preemptions = \
            preemptremoval.countPreemptions(sched, enabled, condlist)
    assert (end_npcs + end_preemptions) == end_context, "sanity fail"

    print "Start:\n\tblocks:", start_blocks, 
    print "\n\tcontext switches:", start_context, "\n\t\tpreemptions:",
    print start_preemptions, "\n\t\tnon-preemptive switches:", start_npcs

    print
    print "End:\n\tblocks:", end_blocks,
    print "\n\tcontext switches:", end_context, "\n\t\tpreemptions:",
    print end_preemptions, "\n\t\tnon-preemptive switches:", end_npcs


if __name__ == "__main__":
    main()


