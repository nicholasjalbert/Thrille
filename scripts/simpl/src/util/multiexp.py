# This python script will run our simplification algorithm multiple times
# for a single benchmark

import sys
import os
import copy
import shutil
import pickle
import subprocess
import traceback
import time
import signal

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "tinertia"))
import tinertia 

# http://stackoverflow.com/questions/616645/how-do-i-duplicate-sys-stdout-to-a-log-file-in-python
class Tee(object):
    def __init__(self, name, mode):
        self.file = open(name, mode)
        self.stdout = sys.stdout
        sys.stdout = self

    def __del__(self):
        sys.stdout = sys.__stdout__
        self.file.close()

    def write(self, data):
        self.file.write(data)
        self.stdout.write(data)


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
    assert os.path.exists("thrille-randomactive"), "Need race data"


def executionFail(signum, frame):
    print "Benchmark timeout. Retry..."


def getNewErrorSchedule(bin_save, bin_file, bin_flags):
    liblockrace = os.path.join(bin_save, "liblockrace.so")
    librandact = os.path.join(bin_save, "librandact.so") 
    libstrictserial = os.path.join(bin_save, "libstrictserial.so") 
    libserializer = os.path.join(bin_save, "libserializer.so") 
    assert os.path.exists(liblockrace)
    assert os.path.exists(librandact)
    assert os.path.exists(libstrictserial)
    binarydir, bin = os.path.split(bin_file)
    cwd = os.getcwd()

    if "thrille-randomactive" not in os.listdir(cwd):
        assert False, "no lock race data to randomactive test"
    assert os.path.exists("thrille-randomactive")
    execution_count = 0;
    while True:
        if os.path.exists("thrille-sched"):
            os.remove("thrille-sched")
        if os.path.exists("thrille-relaxed-sched"):
            os.remove("thrille-relaxed-sched")
        execution_count += 1

        logout = open("randact.log", "w")
        
        os.environ["LD_PRELOAD"] = librandact
        args = [bin_file] + bin_flags
        print "Executing: ", args
        signal.signal(signal.SIGALRM, executionFail)
        signal.alarm(30)
        s_time = time.time()
        try:
            subprocess.call(args, stdout=logout, stderr=logout)
        except OSError:
            continue
        e_time = time.time()
        signal.alarm(0)
        print "Time to execute:", e_time - s_time
        del os.environ["LD_PRELOAD"]
        logout.close()
        s = schedule.Schedule()
        try:
            s_time = time.time()
            s = schedule.Schedule("my-schedule")
            e_time = time.time()
            total_time = e_time - s_time
            if total_time > 20:
                print "Schedule took too long to read, retrying..."
                continue
            print "Took", total_time, "to read in schedule"
        except AssertionError:
            print "Problem with capturing schedule"
            t, v, tb = sys.exc_info()
            traceback.print_tb(tb)
            s = schedule.Schedule()
        if s.error is not None:
            trace_path = os.path.join(cwd, "my-schedule")
            thrille_sched = os.path.join(cwd, "thrille-sched")
            shutil.copy(trace_path, thrille_sched)
            os.environ["LD_PRELOAD"] = libstrictserial
            logout = open("tracesanity.log", "w")
            args = [bin_file] + bin_flags
            subprocess.call(args, stdout=logout, stderr=logout)
            del os.environ["LD_PRELOAD"]
            logout.close()
            #os.remove(thrille_sched)
            os.remove("tracesanity.log")
            s = schedule.Schedule("my-schedule")
            if s.error is not None:
                return trace_path, execution_count
            


def resetRun(my_save_dir):
    assert os.path.exists(my_save_dir)
    os.remove(my_save_dir + "/start-sched")
    if os.path.exists(my_save_dir + "/SIMPL.LOG"):
        os.remove(my_save_dir + "/SIMPL.LOG")
    os.remove(my_save_dir + "/start-relaxed-sched")
    os.remove(my_save_dir + "/start-trace")
    os.rmdir(my_save_dir)
    assert not os.path.exists(my_save_dir)

def main():
    checkEnvironment()
    times_to_repeat = int(sys.argv[1])
    save_directory = os.path.abspath(sys.argv[2])
    save_directory = os.path.normpath(save_directory)
    binary_file = os.path.abspath(sys.argv[3])
    binary_file = os.path.normpath(binary_file)
    tr = os.environ.get('THRILLE_ROOT')

    fout = open(os.path.join(save_directory, "simpl-runstat"), "w")
    my_bin_save = os.path.join(save_directory, "bin")
    my_error_save = os.path.join(save_directory, "error")
    my_misc_save = os.path.join(save_directory, "etc")
    script_src = os.path.join(tr, "scripts")

    #make sure compile is current
    compile_script = os.path.join(script_src, "simpl", "compile.sh")
    if (os.path.exists(compile_script)):
        print
        print "Multiexp: Compiling Thrille"
        args = [compile_script]
        return_val = subprocess.call(args)
        assert return_val == 0
        print "Multiexp: Finished Compiling Thrille"
        print

    
    os.mkdir(my_bin_save)
    os.mkdir(my_error_save)
    os.mkdir(my_misc_save)
    errout = open(os.path.join(my_error_save, "error.log"), "w")
    binpath, binname = os.path.split(binary_file)
    bench_root, tmp = os.path.split(binpath)
    bench_make = os.path.join(bench_root, "Makefile")
    bench_src = os.path.join(bench_root, "src")


    #try to compile benchmark
    if os.path.exists(bench_make) and "-e1" in binname:
        dirsave = os.getcwd()
        print
        print "Multiexp: Compiling Benchmark with llvmerr1"
        os.chdir(bench_root)
        return_val = subprocess.call(["make", "-f", "Makefile", "llvmerr1"])
        assert return_val == 0
        print "Multiexp: Finished Compiling Benchmark with llvmerr1"
        print
        os.chdir(dirsave)
    
    shutil.copy(binary_file, os.path.join(my_bin_save, binname))
    shutil.copy(os.path.join(tr, "bin", "libserializer.so"), \
            os.path.join(my_bin_save, "libserializer.so"))
    shutil.copy(os.path.join(tr, "bin", "libstrictserial.so"), \
            os.path.join(my_bin_save, "libstrictserial.so"))
    shutil.copy(os.path.join(tr, "bin", "librelaxedserial.so"), \
            os.path.join(my_bin_save, "librelaxedserial.so"))
    shutil.copy(os.path.join(tr, "bin", "librelaxedtester.so"), \
            os.path.join(my_bin_save, "librelaxedtester.so"))
    shutil.copy(os.path.join(tr, "bin", "libaddrserial.so"), \
            os.path.join(my_bin_save, "libaddrserial.so"))
    shutil.copy(os.path.join(tr, "bin", "librandomschedule.so"), \
            os.path.join(my_bin_save, "librandomschedule.so"))
    shutil.copy(os.path.join(tr, "bin", "librandact.so"), \
            os.path.join(my_bin_save, "librandact.so"))
    shutil.copy(os.path.join(tr, "bin", "libracer.so"), \
            os.path.join(my_bin_save, "libracer.so"))
    shutil.copy(os.path.join(tr, "bin", "liblockrace.so"), \
            os.path.join(my_bin_save, "liblockrace.so"))

    shutil.copy("thrille-randomactive", \
            os.path.join(my_misc_save, "thrille-randomactive"))

    my_src_save = os.path.join(save_directory, "src")
    os.mkdir(my_src_save)
    thrille_src = os.path.join(tr, "src")

    #turn off debug printing in thrille framework
    serializer_config = os.path.join(thrille_src, "serializer", "config")
    print_config = os.path.join(serializer_config, "thrille-print")
    assert os.path.exists(print_config)
    print_out = open(print_config, "w")
    print_out.write("0")
    print_out.close()
    
    if os.path.islink(os.path.join(thrille_src, "llvm26")):
        shutil.copytree(thrille_src, os.path.join(my_src_save, "src"),\
                symlinks=True)
    shutil.copytree(script_src, os.path.join(my_src_save, "scripts"))
   
    # reload local schedule and tinertia from the experimental save dir
    # to allow for modification concurrent with experiment
    # runnning, this might be unnecessary
    script_import_dir = os.path.join(my_src_save, "scripts",\
            "simpl", "src")
    assert os.path.exists(script_import_dir)

    script_common = os.path.join(script_import_dir, "common")
    assert os.path.exists(script_common)

    script_tinertia = os.path.join(script_import_dir, "tinertia")
    assert os.path.exists(script_tinertia) 

    sys.path.append(script_common)
    sys.path.append(script_tinertia)

    argv_script_directory, script_name = os.path.split(sys.argv[0])
    assert argv_script_directory in sys.path
    sys.path.remove(argv_script_directory)
    reload(schedule)
    reload(tinertia)

    if os.path.exists(bench_src):
        shutil.copytree(bench_src, os.path.join(my_src_save, "bench"))

    if os.path.exists(bench_make):
        shutil.copy(bench_make, os.path.join(my_misc_save, "Makefile"))
    
    
    fout.write("Command that was run:\n")
    for x in sys.argv:
        fout.write(x + " ")
    fout.write("\n\n")

    #lists for tracking statistics
    start_list = []
    end_list = []
    i = 0
    while i < times_to_repeat:
        print "**RUN:", i
        my_save_dir = ""
        if (i < 10):
            my_save_dir = os.path.join(save_directory, "run00" + str(i))
        elif (i < 100):
            my_save_dir = os.path.join(save_directory, "run0" + str(i))
        else:
            my_save_dir = os.path.join(save_directory, "run" + str(i))
        os.mkdir(my_save_dir)


        my_tee = Tee(os.path.join(my_save_dir, "SIMPL.LOG"), "w")

        err_trace_start_time = time.time()
        error_trace, err_execs = getNewErrorSchedule(my_bin_save, \
                binary_file, sys.argv[4:])
        err_trace_end_time = time.time()

        print  
        print "Time to find error trace:", 
        print (err_trace_end_time - err_trace_start_time), "sec"
        print "Executions to find error trace:", err_execs
        print  


        start_trace = os.path.join(my_save_dir, "start-trace")
        shutil.copy(error_trace, start_trace)
        startsched = schedule.Schedule(error_trace)
        start_save = os.path.join(my_save_dir, "start-sched")
        start_relax = os.path.join(my_save_dir, "start-relaxed-sched")
        pickle.dump(startsched, open(start_save, "w"))
        startsched.outputRelaxedSchedule(start_relax)

        try:
            libaddrserial = os.path.join(my_bin_save, "libaddrserial.so")
            libtester = os.path.join(my_bin_save, "librelaxedtester.so")
            s = tinertia.Simplifier(tr, libaddrserial, libtester, error_trace,\
                    binary_file, sys.argv[4:])


            #libstrict = os.path.join(my_bin_save, "libstrictserial.so")
            #s = tinertia.Simplifier(tr, libstrict, libstrict, error_trace, \
            #        binary_file, sys.argv[4:])
        except AssertionError:
            print "***ERROR: inconsistent schedule, restart"
            t, v, tb = sys.exc_info()
            traceback.print_tb(tb)
            errout.write("Error trace " + str(i) + " was inconsistent:\n")
            errout.write("We thought we had error: " + str(startsched.error))
            errout.write("\tRetrying--error trace is now err-trace" + str(i))
            errout.write("\n")
            dst = os.path.join(my_error_save, "err-trace" + str(i))
            shutil.copy(start_trace, dst)
            resetRun(my_save_dir)
            continue

        donesched = schedule.Schedule()
        simplify_time = time.time()
        try:
            starttime = time.time()
            donesched = s.simplify()
            endtime = time.time()
            simplify_time = endtime - starttime
            assert donesched != schedule.Schedule()
        except AssertionError:
            print "***ERROR: simplification, restart"
            t, v, tb = sys.exc_info()
            traceback.print_tb(tb)
            errout.write("Simplify Fail" + str(i) + "\n")
            errout.write("\terror: " + str(startsched.error) + "\n\n")
            dst = os.path.join(my_error_save, "err-trace-simp" + str(i))
            shutil.copy(start_trace, dst)
            resetRun(my_save_dir)
            continue

        assert donesched.error == startsched.error
        assert donesched.addrlist == startsched.addrlist
        #assert donesched.getScheduleLength() <= startsched.getScheduleLength()
        
        start_list.append(copy.deepcopy(startsched))
        end_list.append(copy.deepcopy(donesched))

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
        print donestr,
        print
        print "Number of Iterations:", s.iterations
        print "Number of Executions:", s.executions
        print "Time (sec):", simplify_time
        print "Number of empty schedules:", s.empty_schedules
        print "Number of generated schedules which show our bug:", 
        print s.our_bug_count
        print "Number of generated schedules which show a different bug:",
        print s.other_bug_count
        print "Number of generated schedules which show no bug:",
        print s.no_bug_count
        print
        print "Avg threads enabled at all context switches:",
        print donesched.getAvgThreadsEnabledAtCS()
        print "Avg threads enabled at non-preemptive context switches:",
        print donesched.getAvgThreadsEnabledAtNons()
        print "Avg threads enabled at preemptive context switches:",
        print donesched.getAvgThreadsEnabledAtPreempts()
        print "Avg threads enabled at all scheduling points:",
        print donesched.getAvgThreadsEnabledAtAll()
        print

        key_set = s.transform_stat_blocks.keys()
        key_set.sort()
        for x in key_set:
            print "Scheduling points removed by", x, "s:",
            print s.transform_stat_blocks[x]
            print "Threads removed by", x, "t:",
            print s.transform_stat_threads[x]
            print "Context Switches removed by", x, "c:",
            print s.transform_stat_ctxt[x]
            print "Non-Preemptive Context Switches removed by", x, "n:",
            print s.transform_stat_nonpre[x]
            print "Preemptive Context Switches removed by", x, "p:",
            print s.transform_stat_pre[x]
            print 

        del my_tee
        del sys.stdout

        fout.write("**RUN " + str(i) + "\n")
        sys.stdout = fout
        print "Error:", donesched.error
        print
        print "Start Schedule:"
        print startstr
        print
        print "Simplified Schedule:"
        print donestr,
        print
        print "Number of Iterations:", s.iterations
        print "Number of Executions:", s.executions
        print "Time (sec):", simplify_time
        print "Number of empty schedules:", s.empty_schedules
        print "Number of generated schedules which show our bug:", 
        print s.our_bug_count
        print "Number of generated schedules which show a different bug:",
        print s.other_bug_count
        print "Number of generated schedules which show no bug:",
        print s.no_bug_count
        print
        print "Avg threads enabled at all context switches:",
        print donesched.getAvgThreadsEnabledAtCS()
        print "Avg threads enabled at non-preemptive context switches:",
        print donesched.getAvgThreadsEnabledAtNons()
        print "Avg threads enabled at preemptive context switches:",
        print donesched.getAvgThreadsEnabledAtPreempts()
        print "Avg threads enabled at all scheduling points:",
        print donesched.getAvgThreadsEnabledAtAll()
        print 
        
        key_set = s.transform_stat_blocks.keys()
        key_set.sort()
        for x in key_set:
            print "Scheduling points removed by", x, "s:",
            print s.transform_stat_blocks[x]
            print "Threads removed by", x, "t:",
            print s.transform_stat_threads[x]
            print "Context Switches removed by", x, "c:",
            print s.transform_stat_ctxt[x]
            print "Non-Preemptive Context Switches removed by", x, "n:",
            print s.transform_stat_nonpre[x]
            print "Preemptive Context Switches removed by", x, "p:",
            print s.transform_stat_pre[x]
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


