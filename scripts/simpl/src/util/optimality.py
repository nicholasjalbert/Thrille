#Calculates distance from optimality for our benches with optimal traces

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

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule

def check_environment():
    if len(sys.argv) != 2:
        print "usage: python optimality.py",
        print "[results dir]"
        print
        print "purpose: for each benchmark with optimal traces, ",
        print "this script records how close to optimal we were for",
        print "each simplified trace in [results dir]"
        print
        sys.exit(1)
    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"


def get_optimal_schedules(opt_dir):
    assert os.path.exists(opt_dir)
    schedule_list = []
    for file in os.listdir(opt_dir):
        if "trace" in file:
            trace_path = os.path.join(opt_dir, file)
            tmp_sched = schedule.Schedule(trace_path)
            for s in schedule_list:
                assert s.error != tmp_sched.error
            schedule_list.append(tmp_sched)
    return schedule_list


def get_simplified_schedules(exp_dir):
    assert os.path.exists(exp_dir)
    schedule_list = []
    for item in os.listdir(exp_dir):
        if "run" in item:
            run_dir = os.path.join(exp_dir, item)
            if not os.path.isdir(run_dir):
                continue
            assert os.path.exists(run_dir)
            simp_sched = os.path.join(run_dir, "simpl-sched")
            if not os.path.exists(simp_sched):
                print "Could not find", simp_sched, ". Onward!"
                continue
            assert os.path.exists(simp_sched)
            tmp_sched = pickle.load(open(simp_sched, "r"))
            schedule_list.append(tmp_sched)
    return schedule_list


def calculate_gap(simpl, opt, dict):
    size_diff = simpl.getScheduleLength() - opt.getScheduleLength()
    if not (size_diff in dict["size"].keys()):
        dict["size"][size_diff] = 0
    dict["size"][size_diff] += 1

    ctxt_diff = simpl.getContextSwitches() - opt.getContextSwitches()
    #assert ctxt_diff > 0
    if not (ctxt_diff in dict["ctxt"].keys()):
        dict["ctxt"][ctxt_diff] = 0
    dict["ctxt"][ctxt_diff] += 1


def optimality_gap(simpl, optimals, dict):
    for opt in optimals:
        if opt.error == simpl.error:
            calculate_gap(simpl, opt, dict)
            return
    print "Could not find this error:", simpl.error
    assert False 


def optimality_check(opt_dir, exp_dir):
    optimal_schedules = get_optimal_schedules(opt_dir)
    simplified_schedules = get_simplified_schedules(exp_dir)

    opt_dict = {}
    opt_dict["size"] = {}
    opt_dict["ctxt"] = {}
    print exp_dir
    for simpl_sched in simplified_schedules:
        optimality_gap(simpl_sched, optimal_schedules, opt_dict)
    
    return opt_dict

def print_results(dir, opt_dict):
        print dir
        #print "Size (scheduling points) (DEPRECATED)"
        size_keys = opt_dict["size"].keys()
        size_keys.sort()
        for x in size_keys:
            if x == 0:
                pass
                #print "Optimal:", opt_dict["size"][0]
            else:
                pass
                #print x, "more than optimal:", opt_dict["size"][x]

        print 
        print "Context Switches"
        ctxt_keys = opt_dict["ctxt"].keys()
        ctxt_keys.sort()
        for x in ctxt_keys:
            if x == 0:
                print "Optimal:", opt_dict["ctxt"][0]
            else:
                print x, "more than optimal:", opt_dict["ctxt"][x]

        print 
        print "----------------------------"

def output_csv(fout, dir, opt_dict):
    fout.write(dir)

    for x in range(0,6):
        if not (x in opt_dict["ctxt"].keys()):
            fout.write(",0")
        else:
            fout.write("," + str(opt_dict["ctxt"][x]))
    fout.write("\n")





def main():
    check_environment()
    tr = os.environ.get('THRILLE_ROOT')
    bench_root = os.path.join(tr, "benchmarks", "simpl")
    exp_name = sys.argv[1]
    assert os.path.exists(bench_root)
    
    fout = open("optimality.csv", "w")
    fout.write("Benchmark,Optimal,1 more,2 more,3 more,4 more,5 more\n")

    for dir in os.listdir(bench_root):
        opt_dir = os.path.join(bench_root, dir, "opt")
        exp_dir = os.path.join(bench_root, dir, "exp", exp_name)
        if not os.path.exists(opt_dir):
            continue
        if not os.path.exists(exp_dir):
            continue
        
        try:
            opt_dict = optimality_check(opt_dir, exp_dir)
        except AssertionError:
            print "====================="
            print "Problem with benchmark", dir
            print "====================="
            continue


        print_results(dir, opt_dict)
        output_csv(fout, dir, opt_dict)

    fout.close()




   

if __name__ == "__main__":
    main()

