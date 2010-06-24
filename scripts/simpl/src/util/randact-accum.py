# quick and dirty counter to tabulate randomactive results

import os
import sys
import copy
import subprocess
import shutil
import pickle
import time 


def getTimeAndExecutionsToFindBug(experiment_log):
    count_list = []
    time_list = []
    while len(experiment_log) > 0:
        line = experiment_log.pop(0)
        count_accum = 0
        time_accum = 0
        if "**RUN:" in line:
            while len(experiment_log) > 0:
                potential_exec = experiment_log.pop(0) 
                if "Executing:" in potential_exec:
                    count_accum += 1
                    time_to_exec = experiment_log.pop(0)
                    assert "Time to execute:" in time_to_exec
                    time_accum += float(time_to_exec.split(":")[1])
                if "Schedule Statistics" in potential_exec:
                    count_list.append(count_accum)
                    time_list.append(time_accum)
                    break

        if "**ERROR:" in line:
            count_list.pop()
            time_list.pop()
    assert len(count_list) == len(time_list)
    return (count_list, time_list)

def getAvg(number_list):
    return float(sum(number_list))/float(len(number_list))
                   
def main():
    
    tr = os.environ.get('THRILLE_ROOT')
    if len(sys.argv) < 2:
        print "Usage: python randact-accum.py [exp dir name]"
        sys.exit()
    exp_name = sys.argv[1]
    assert tr is not None
    simpl_dir = os.path.join(tr, "benchmarks", "simpl")
    assert os.path.exists(simpl_dir)

    data = {}

    for dir in os.listdir(simpl_dir):
        exp_path = os.path.join(simpl_dir, dir, \
                "exp", exp_name)
        log_path = os.path.join(exp_path, "exp.log")
        if not os.path.exists(log_path):
            continue
        experiment_log = open(log_path, "r").readlines()
        data[dir] = getTimeAndExecutionsToFindBug(experiment_log)

    print
    print "Results for Random Active (exp:", exp_name, "):"
    for x in data:
        print x, ":"
        count_list, time_list = data[x]
        #print count_list
        #print time_list
        print "\treporting from", len(count_list), "runs"
        print "\tAvg executions to find bug:", getAvg(count_list)
        print "\tAvg time to find bug:", getAvg(time_list)

main()
