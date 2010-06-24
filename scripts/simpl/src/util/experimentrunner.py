#Runs the multiexp.py scripts for each of our benchmarks

import sys
import os
import copy
import shutil
import pickle
import subprocess
import traceback
import time
import signal
from subprocess import Popen

def checkEnvironment():
    if len(sys.argv) < 3:
        print "usage: python experimentrunner.py [times to execute]",
        print "[results dir]"
        print
        print "purpose: for each benchmark in the internal list, ",
        print "this script generates the [results dir] and runs",
        print "multiexp.py the appropriate number of times"
        print
        print "alternate: python experimentrunner.py -no-bench",
        print "[expected times executed] [results dir]"
        print
        print "purpose: generates statistics for runs in the [results dir]"
        print
        print "alternate: python experimentrunner.py -chess",
        print "[bound] [results dir]"
        print
        print "purpose: executes context bounded MC across the benchmarks"
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert int(sys.argv[1]) > 0, "Nonsensical execution inumber"



def runMeSomeBenchmarks(benchmark_list, param_dict, env_dict, \
        times_to_repeat, save_directory):
    tr = os.environ.get("THRILLE_ROOT")
    assert not (tr is None)

    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    multi_exp = os.path.join(scripts_src, "util",  "multiexp.py")
    assert os.path.exists(benchmark_path)
    assert os.path.exists(multi_exp)

    p1 = None
    p1name = ""
    p2 = None
    p2name = ""
    p3 = None
    p3name = ""
    p4 = None
    p4name = ""
    

    for x in benchmark_list:
        curr_dir = os.getcwd()
        bin_dir = os.path.join(benchmark_path, x, "bin")
        src_dir = os.path.join(benchmark_path, x, "src")
        
        assert os.path.exists(src_dir)

        setup_script = os.path.join(src_dir, "setup.py")
        teardown_script = os.path.join(src_dir, "teardown.py")

        if os.path.exists(setup_script):
            args = ["python", setup_script]
            fnull = open("/dev/null", "w")
            subprocess.call(args, stdout = fnull, stderr = fnull)
            fnull.close()
            
        my_save_dir = os.path.join(benchmark_path, x, "exp", save_directory)
        os.mkdir(my_save_dir)
        my_fout = open(os.path.join(my_save_dir, "exp.log"), "w")
        os.chdir(bin_dir)
        args = ["python", multi_exp, str(times_to_repeat), my_save_dir]
        map(lambda y: args.append(y), param_dict[x].split())
        execution_env = copy.deepcopy(os.environ)

        for k, v in env_dict[x].items():
            print "Adding env variable:",k,"=",v
            execution_env[k] = v

        print "calling:",args

        if p1 is None:
            p1 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p1name = x
        elif p2 is None:
            p2 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p2name = x
        elif p3 is None:
            p3 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p3name = x
        elif p4 is None:
            p4 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p4name = x

        os.chdir(curr_dir)
        time.sleep(30)

        if os.path.exists(teardown_script):
            args = ["python", teardown_script]
            fnull = open("/dev/null", "w")
            subprocess.call(args, stdout = fnull, stderr = fnull)
            fnull.close()


        no_more_exp = True
        while no_more_exp:
            no_more_exp = not (p1 is None)
            no_more_exp = no_more_exp and not (p2 is None)
            no_more_exp = no_more_exp and not (p3 is None)
            no_more_exp = no_more_exp and not (p4 is None)

            if not no_more_exp:
                break

            if not (p1 is None):
                rc = p1.poll()
                if rc is not None:
                    print p1name, "done..."
                    p1 = None
            if not (p2 is None):
                rc = p2.poll()
                if rc is not None:
                    print p2name, "done..."
                    p2 = None
            if not (p3 is None):
                rc  = p3.poll()
                if rc is not None:
                    print p3name, "done..."
                    p3 = None
            if not (p4 is None):
                rc = p4.poll()
                if rc is not None:
                    print p4name, "done..."
                    p4 = None

            time.sleep(30)


    if not (p1 is None):
        p1.wait()
        print p1name, "done..."
        p1 = None
    if not (p2 is None):
        p2.wait()
        print p2name, "done..."
        p2 = None
    if not (p3 is None):
        p3.wait()
        print p3name, "done..."
        p3 = None
    if not (p4 is None):
        p4.wait()
        print p4name, "done..."
        p4 = None


def runChess(benchmark_list, param_dict, env_dict, bound, save_directory):
    tr = os.environ.get("THRILLE_ROOT")
    assert not (tr is None)

    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    chess = os.path.join(scripts_src, "chess", "chess.py")
    assert os.path.exists(benchmark_path)
    assert os.path.exists(chess)

    p1 = None
    p1name = ""
    p2 = None
    p2name = ""
    p3 = None
    p3name = ""
    p4 = None
    p4name = ""
    

    for x in benchmark_list:
        curr_dir = os.getcwd()
        bin_dir = os.path.join(benchmark_path, x, "bin")
        src_dir = os.path.join(benchmark_path, x, "src")
        
        assert os.path.exists(src_dir)

        setup_script = os.path.join(src_dir, "setup.py")
        teardown_script = os.path.join(src_dir, "teardown.py")

        if os.path.exists(setup_script):
            args = ["python", setup_script]
            fnull = open("/dev/null", "w")
            subprocess.call(args, stdout = fnull, stderr = fnull)
            fnull.close()
            
        my_save_dir = os.path.join(benchmark_path, x, "exp", save_directory)
        os.mkdir(my_save_dir)
        my_fout = open(os.path.join(my_save_dir, "exp.log"), "w")
        os.chdir(bin_dir)
        args = ["python", chess, str(bound)]
        bindir, tmp = os.path.split(my_save_dir)
        args.append(os.path.join(bindir, "11182009","run000","start-trace"))
        args.append(my_save_dir)
        map(lambda y: args.append(y), param_dict[x].split())
        
        execution_env = copy.deepcopy(os.environ)
        for k, v in env_dict[x].items():
            print "Adding env variable:",k,"=",v
            execution_env[k] = v

        print "calling:",args

        if p1 is None:
            p1 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p1name = x
        elif p2 is None:
            p2 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p2name = x
        elif p3 is None:
            p3 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p3name = x
        elif p4 is None:
            p4 = Popen(args, stdout=my_fout, stderr=my_fout, env=execution_env)
            print "launching", x
            p4name = x

        os.chdir(curr_dir)
        time.sleep(30)

        if os.path.exists(teardown_script):
            args = ["python", teardown_script]
            fnull = open("/dev/null", "w")
            subprocess.call(args, stdout = fnull, stderr = fnull)
            fnull.close()


        no_more_exp = True
        while no_more_exp:
            no_more_exp = not (p1 is None)
            no_more_exp = no_more_exp and not (p2 is None)
            no_more_exp = no_more_exp and not (p3 is None)
            no_more_exp = no_more_exp and not (p4 is None)

            if not no_more_exp:
                break

            if not (p1 is None):
                rc = p1.poll()
                if rc is not None:
                    print p1name, "done..."
                    p1 = None
            if not (p2 is None):
                rc = p2.poll()
                if rc is not None:
                    print p2name, "done..."
                    p2 = None
            if not (p3 is None):
                rc  = p3.poll()
                if rc is not None:
                    print p3name, "done..."
                    p3 = None
            if not (p4 is None):
                rc = p4.poll()
                if rc is not None:
                    print p4name, "done..."
                    p4 = None

            time.sleep(30)


    if not (p1 is None):
        p1.wait()
        print p1name, "done..."
        p1 = None
    if not (p2 is None):
        p2.wait()
        print p2name, "done..."
        p2 = None
    if not (p3 is None):
        p3.wait()
        print p3name, "done..."
        p3 = None
    if not (p4 is None):
        p4.wait()
        print p4name, "done..."
        p4 = None


def checkMeSomeErrors(benchmark_list, times_to_repeat, save_directory):
    
    tr = os.environ.get("THRILLE_ROOT")
    assert not (tr is None)

    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    multi_exp = os.path.join(scripts_src, "util", "multiexp.py")
    assert os.path.exists(benchmark_path)
    assert os.path.exists(multi_exp)

    print 
    print
    print "Error Checking"
    error_found = False
    benchmark_list.sort()
    for x in benchmark_list:
        my_save = os.path.join(benchmark_path, x, "exp", save_directory)

        if not os.path.exists(my_save):
            print "Benchmark", x, "is not done yet, skipping..."
            continue
        my_error_save = os.path.join(my_save, "error")
        assert os.path.exists(my_error_save)
        err_list = os.listdir(my_error_save)
        num_errs = len(err_list) - 1
        if num_errs != 0:
            print "Benchmark", x, "finished with ERRORS"
            error_found = True
        for z in range(0, times_to_repeat):
            tar_dir = ""
            if z < 10:
                tar_dir = "run00" + str(z)
            elif z < 100:
                tar_dir = "run0" + str(z)
            else:
                tar_dir = run + str(z)
            tar_dir = os.path.join(my_save, tar_dir)
            if not os.path.exists(tar_dir):
                tmp, name = os.path.split(tar_dir)
                print "Benchmark", x, "missing run", name
                error_found = True

    if not error_found:
        print "Experiments completed with no unexpected errors"

def generateMeSomeStatistics(benchmark_list, save_directory, \
        latex1_dict, latex2_dict, latex3_dict):
    tr = os.environ.get("THRILLE_ROOT")
    assert not (tr is None)

    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    multi_exp = os.path.join(scripts_src, "util", "multiexp.py")
    assert os.path.exists(benchmark_path)
    assert os.path.exists(multi_exp)

    print
    print
    print "Generating summary and latex tables"

    tr = os.environ.get("THRILLE_ROOT")
    assert not (tr is None)

    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    multi_exp = os.path.join(scripts_src, "util", "multiexp.py")
    assert os.path.exists(benchmark_path)
    assert os.path.exists(multi_exp)
    
    fsummary = open(save_directory + "-summary", "w")
    ftex1 = open(save_directory + "-latex1", "w")
    ftex2 = open(save_directory + "-latex2", "w")
    ftex3 = open(save_directory + "-chart3.csv", "w")

    benchmark_list.sort()
    for x in benchmark_list:
        exp_dir = os.path.join(benchmark_path, x, "exp", save_directory) 
        if not os.path.exists(exp_dir):
            print "Benchmark", x, "not complete yet, skipping..."
            continue
        simpl_runstat = os.path.join(exp_dir, "simpl-runstat")
        assert os.path.exists(simpl_runstat)

        pystat = os.path.join(scripts_src, "util", "statgenerator.py")
        assert os.path.exists(pystat)

        curr_dir = os.getcwd()
        os.chdir(exp_dir)
        fnull = open("/dev/null", "w")
        args = ["python", pystat, simpl_runstat]
        print "calling", args
        subprocess.call(args, stdout=fnull, stderr=fnull)
        fnull.close()

        simp_summarystat = os.path.join(exp_dir, "simp-summarystat")
        latexchart1_tex = os.path.join(exp_dir, "latexchart1.tex")
        latexchart2_tex = os.path.join(exp_dir, "latexchart2.tex")
        latexchart3_tex = os.path.join(exp_dir, "chart3.csv")

        assert os.path.exists(simp_summarystat)
        assert os.path.exists(latexchart1_tex)
        assert os.path.exists(latexchart2_tex)
        assert os.path.exists(latexchart3_tex)

        fsummary.write(x + "\n")

        sin = open(simp_summarystat, "r").readlines()
        map(lambda y: fsummary.write(y), sin)

        fsummary.write("\n\n")

        ftex1.write(latex1_dict[x])
        map(lambda y: ftex1.write(y), open(latexchart1_tex, "r").readlines())
        ftex1.write("\\hline\n")

        ftex2.write(latex2_dict[x])
        map(lambda y: ftex2.write(y), open(latexchart2_tex, "r").readlines())
        ftex2.write("\\hline\n")
        
        map(lambda y: ftex3.write(y), open(latexchart3_tex, "r").readlines())

    fsummary.close()
    ftex1.close()
    ftex2.close()
    ftex3.close()


# add a new benchmark by:
#       -adding it to benchmark_list
#       -adding how you execute it to param_dict
#       -add any environment variables to env_dict
#       -adding latex table 1 heading to latex1_dict
#       -adding latex table 2 heading to latex2_dict
#       -adding latex table 3 heading to latex3_dict
def main():
    run_benchmarks = True 
    chess_check = False
    times_to_repeat = 0
    bound = 0
    save_directory = None

    if len(sys.argv) > 1 and "-no-bench" in sys.argv[1]:
        run_benchmarks = False
        assert len(sys.argv) == 4    
        times_to_repeat = int(sys.argv[2])
        save_directory = sys.argv[3]
    elif len(sys.argv) > 1 and "-chess" in sys.argv[1]:
        run_benchmarks = False
        chess_check = True
        assert len(sys.argv) == 4    
        bound = int(sys.argv[2])
        save_directory = sys.argv[3]
    else:
        checkEnvironment()    
        times_to_repeat = int(sys.argv[1])
        save_directory = sys.argv[2]

    tr = os.environ.get('THRILLE_ROOT')
    benchmark_path = os.path.join(tr, "benchmarks", "simpl")
    scripts_src = os.path.join(tr, "scripts", "simpl", "src") 
    multi_exp = os.path.join(scripts_src, "util",  "multiexp.py")

    assert os.path.exists(benchmark_path)
    assert os.path.exists(multi_exp)
    
    # BENCHMARK LIST    
    benchmark_list = [] 
    benchmark_list.append("bbuf")
    benchmark_list.append("blackscholes")
    benchmark_list.append("canneal")
    benchmark_list.append("ctrace")
    benchmark_list.append("pbzip2")
    benchmark_list.append("pfscan")
    benchmark_list.append("dedup")
    benchmark_list.append("swarm")
    benchmark_list.append("streamcluster")
    benchmark_list.append("bzip2")
    benchmark_list.append("x264")
    benchmark_list.append("vips")

    # PARAM DICT -- how to call each benchmark
    param_dict = {}
    param_dict["bbuf"] = "bb-e1"
    param_dict["blackscholes"] = "blackscholes-e1 8 32"
    param_dict["canneal"] = "canneal-e1 12 100 100 ../inputs/10.nets"
    param_dict["ctrace"] = "ctrace-e1 10"
    param_dict["pbzip2"] = "pbzip2-e1 -f -k -p5 -o ../inputs/smallinput"
    param_dict["pfscan"] = "pfscan-e1 -n8 jalbert /home/jalbert/pfscan_test"
    dedup_str = "dedup-e1 -c -p -f -t 4 -i ../inputs/nick.dat -o /dev/null"
    param_dict["dedup"] = dedup_str
    param_dict["swarm"] = "swarm-e1"
    sc_str = "streamcluster-e1 2 2 1 10 10 5 none ../obj/output.txt 4"
    param_dict["streamcluster"] = sc_str
    param_dict["bzip2"] = "bzip2smp-e1 -1 -p4 ../inputs/smallinput"
    param_dict["x264"] = "x264 --quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse 8x8.i4x4 --threads 4 -o ../obj/eledream.264 ../inputs/eledream_640x360_8.y4m"
    assert os.environ.get("IM_CONCURRENCY") == "4"
    param_dict["vips"] = "vips-e1 im_benchmark ../inputs/barbados_256x288.v ../obj/output.v"

    
    # ENVIRONMENT DICT -- environment variables that must be set during run 
    env_dict = {}
    env_dict["bbuf"] = {}
    env_dict["blackscholes"] = {}
    env_dict["canneal"] = {}
    env_dict["ctrace"] = {}
    env_dict["pbzip2"] = {}
    env_dict["pfscan"] = {}
    env_dict["dedup"] = {}
    env_dict["swarm"] = {}
    env_dict["streamcluster"] = {"MALLOC_CHECK_":"2"}
    env_dict["bzip2"] = {}
    env_dict["x264"] = {}
    env_dict["vips"] = {"IM_CONCURRENCY":"4"}

    # LATEX1 DICT -- header for generating table 1
    latex1_dict = {}
    latex1_dict["bbuf"] = "boundedBuffer & 255 &\n"
    latex1_dict["blackscholes"] = "blackscholes & 919 &\n"
    latex1_dict["bzip2"] = "bzip2 & 4294 &\n"
    latex1_dict["canneal"] = "canneal & 2822 &\n"
    latex1_dict["ctrace"] = "ctrace & 763 &\n"
    latex1_dict["dedup"] = "dedup & 2571 &\n"
    latex1_dict["pbzip2"] = "pbzip2 & 1489 &\n"
    latex1_dict["pfscan"] = "pfscan & 750 &\n"
    latex1_dict["streamcluster"] = "streamcluster & 1250 &\n"
    latex1_dict["swarm"] = "swarm & 1636 &\n"
    latex1_dict["x264"] = "x264 & 37739 &\n"
    latex1_dict["vips"] = "vips & 123385 &\n"
    
    # LATEX2 DICT -- header for generating table 1
    latex2_dict = {}
    latex2_dict["bbuf"] = "boundedBuffer & deadlock &\n"
    latex2_dict["blackscholes"] = "blackscholes & deadlock &\n"
    latex2_dict["bzip2"] = "bzip2 & segfault &\n"
    latex2_dict["canneal"] = "canneal & deadlock &\n"
    latex2_dict["ctrace"] = "ctrace & deadlock &\n"
    latex2_dict["dedup"] = "dedup & segfault &\n"
    latex2_dict["pbzip2"] = "pbzip2 & segfault &\n"
    latex2_dict["pfscan"] = "pfscan & segfault &\n"
    latex2_dict["streamcluster"] = "streamcluster & segfault &\n"
    latex2_dict["swarm"] = "swarm & assert fail &\n"
    latex2_dict["x264"] = "x264 & deadlock &\n"
    latex2_dict["vips"] = "vips & assert fail / deadlock &\n"

    # LATEX3 DICT -- header for generating table 1
    latex3_dict = {}
    latex3_dict["bbuf"] = "boundedBuffer & \n"
    latex3_dict["blackscholes"] = "blackscholes & \n"
    latex3_dict["bzip2"] = "bzip2 & \n"
    latex3_dict["canneal"] = "canneal & \n"
    latex3_dict["ctrace"] = "ctrace & \n"
    latex3_dict["dedup"] = "dedup & \n"
    latex3_dict["pbzip2"] = "pbzip2 & \n"
    latex3_dict["pfscan"] = "pfscan & \n"
    latex3_dict["streamcluster"] = "streamcluster & \n"
    latex3_dict["swarm"] = "swarm & \n"
    latex3_dict["x264"] = "x264 & \n"
    latex3_dict["vips"] = "vips & \n"

    for x in benchmark_list:
        tmp_dir = os.path.join(benchmark_path, x)
        assert os.path.exists(tmp_dir)
        bin_dir = os.path.join(tmp_dir, "bin")
        assert os.path.exists(bin_dir)
        assert os.path.exists(os.path.join(bin_dir, "thrille-randomactive"))


    
    if run_benchmarks:
        runMeSomeBenchmarks(benchmark_list, param_dict, env_dict, \
                times_to_repeat, save_directory)

    if chess_check:
        runChess(benchmark_list, param_dict, env_dict, bound, save_directory)
        return

    checkMeSomeErrors(benchmark_list, times_to_repeat, save_directory)

    generateMeSomeStatistics(benchmark_list, save_directory, \
            latex1_dict, latex2_dict, latex3_dict)

    

if __name__ == "__main__":
    main()

