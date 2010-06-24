# This python script will run our simplification (fwd rev) algorithm multiple times

import sys
import os
import copy
import shutil
import fwdrev
import simpl
import pickle
import subprocess



def checkEnvironment():
    if len(sys.argv) < 4:
        print "usage: python multiexp.py [times to execute] [results dir]",
        print "[binary under test] [binary flags]"
        print
        print "generates an input schedule and then performs the ",
        print "iterative forward-reverse algorithm on this schedule",
        print "and saves the results"
        print
        print "Output files:"
        print "\tstart-sched: the buggy start schedule (pickled object)"
        print "\tsimpl-sched: the simplified schedule (pickled object)"
        print "\tstart-relaxed-sched"
        print "\tsimpl-relaxed-sched"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert int(sys.argv[1]) > 0, "Nonsensical execution time"
    assert os.path.exists(sys.argv[2]), "Results directory does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"


def getNewErrorSchedule(tr, bin_file, bin_flags):
    liblockrace = os.path.join(tr, "bin/liblockrace.so")
    librandact = os.path.join(tr, "bin/librandact.so") 
    assert os.path.exists(liblockrace)
    assert os.path.exists(librandact)
    binarydir, bin = os.path.split(bin_file)
    curr_dir = os.getcwd()
    if binarydir != '':
        os.chdir(binarydir)

    bin = os.path.join(".", bin)

    if "thrille-randomactive" not in os.listdir(os.getcwd()):
        assert False, "no lock race data to randomactive test"
    assert os.path.exists("thrille-randomactive")
    count = 0;
    while True:
        if os.path.exists("thrille-sched"):
            os.remove("thrille-sched")
        if os.path.exists("thrille-relaxed-sched"):
            os.remove("thrille-relaxed-sched")
        count += 1
        if count > 1000:
            raw_input("1000 iterations with no error--continue?")
            count = 0

        logout = open("randact.log", "w")

        os.environ["LD_PRELOAD"] = librandact
        args = [bin] + bin_flags
        subprocess.call(args, stdout=logout, stderr=logout)
        del os.environ["LD_PRELOAD"]
        logout.close()
        s = fwdrev.Schedule("my-schedule")
        if s.error is not None:
            os.chdir(curr_dir)
            return os.path.join(binarydir, "my-schedule")


def main():
    checkEnvironment()
    times_to_repeat = int(sys.argv[1])
    save_directory = sys.argv[2]
    binary_file = sys.argv[3]
    tr = os.environ.get('THRILLE_ROOT')
    fout = open(os.path.join(save_directory, "simpl-runstat"), "w")
    errout = open(os.path.join(save_directory, "error.log"), "w")
    my_bin_save = os.path.join(save_directory, "bin")
    os.mkdir(my_bin_save)
    tmppath, binname = os.path.split(binary_file)
    shutil.copy(binary_file, os.path.join(my_bin_save, binname))
    shutil.copy(os.path.join(tr, "bin", "libserializer.so"), \
            os.path.join(my_bin_save, "libserializer.so"))
    shutil.copy(os.path.join(tr, "bin", "libstrictserial.so"), \
            os.path.join(my_bin_save, "libstrictserial.so"))
    shutil.copy(os.path.join(tr, "bin", "librelaxedserial.so"), \
            os.path.join(my_bin_save, "librelaxedserial.so"))
    shutil.copy(os.path.join(tr, "bin", "librandomschedule.so"), \
            os.path.join(my_bin_save, "librandomschedule.so"))
    shutil.copy(os.path.join(tr, "bin", "librandact.so"), \
            os.path.join(my_bin_save, "librandact.so"))
    shutil.copy(os.path.join(tr, "bin", "librace.so"), \
            os.path.join(my_bin_save, "librace.so"))
    shutil.copy(os.path.join(tr, "bin", "liblockrace.so"), \
            os.path.join(my_bin_save, "liblockrace.so"))

    #figure out how to remove svn
    #os.mkdir(os.path.join(save_directory, "src"))

    #shutil.copytree(os.path.join(tr, "src"), \
    #        os.path.join(save_directory,"src","src")) \

    #shutil.copytree(os.path.join(tr, "scripts"), \
    #        os.path.join(save_directory,"src","scripts"))
    fout.write("Command that was run:\n")
    for x in sys.argv:
        fout.write(x + " ")
    fout.write("\n\n")

    #lists for tracking statistics
    start_list = []
    end_list = []
    i = 0
    while i < times_to_repeat:
        print "**EXPERIMENT", i
        my_save_dir = ""
        if (i < 10):
            my_save_dir = os.path.join(save_directory, "run0" + str(i))
        else:
            my_save_dir = os.path.join(save_directory, "run" + str(i))
        os.mkdir(my_save_dir)

        error_trace = getNewErrorSchedule(tr, binary_file, sys.argv[4:])
        start_trace = os.path.join(my_save_dir, "start-trace")
        shutil.copy(error_trace, start_trace)
        startsched = fwdrev.Schedule(error_trace)
        start_list.append(copy.deepcopy(startsched))
        start_save = os.path.join(my_save_dir, "start-sched")
        start_relax = os.path.join(my_save_dir, "start-relaxed-sched")
        pickle.dump(startsched, open(start_save, "w"))
        startsched.outputRelaxedSchedule(start_relax)

        s = simpl.Simplifier(tr, error_trace, binary_file, sys.argv[4:])
        donesched = fwdrev.Schedule()
        attempts = 0
        while attempts < 3:
            try:
                donesched = s.simplify()
                assert donesched != fwdrev.Schedule()
                break
            except AssertionError:
                donesched = fwdrev.Schedule()
                errout.write("Retrying Simplify of Iteration " + str(i) + "\n")
                errout.write("\terror: " + startsched.error + "\n\n")
                attempts += 1

        end_list.append(copy.deepcopy(donesched))
        assert donesched.error == startsched.error
        assert donesched.addrlist == startsched.addrlist
        assert donesched.getScheduleLength() <= startsched.getScheduleLength()

        assert len(start_list) == len(end_list)
        
        startstr = startsched.getSummaryInfo()
        donestr = donesched.getSummaryInfo()
        print
        print "Error:", donesched.error
        print
        print "Start Schedule:"
        print startstr
        print
        print "Simplified Schedule:"
        print donestr
        print

        tmpout = open(os.path.join(my_save_dir, "README"), "w")
        sys.stdout = tmpout 
        print "Error:", donesched.error
        print
        print "Start Schedule:"
        print startstr
        print
        print "Simplified Schedule:"
        print donestr
        print
        sys.stdout.flush()
        sys.stdout = sys.__stdout__
        tmpout.close()


        fout.write("**RUN " + str(i) + "\n")
        sys.stdout = fout
        print "Error:", donesched.error
        print
        print "Start Schedule:"
        print startstr
        print
        print "Simplified Schedule:"
        print donestr
        print
        fout.write("\n")
        sys.stdout.flush()
        sys.stdout = sys.__stdout__
        i += 1

        simpl_save = os.path.join(my_save_dir, "simpl-sched")
        simpl_relax = os.path.join(my_save_dir, "simpl-relaxed-sched")
        pickle.dump(donesched, open(simpl_save, "w"))
        donesched.outputRelaxedSchedule(simpl_relax)

    fout.close()



if __name__ == "__main__":
    main()


