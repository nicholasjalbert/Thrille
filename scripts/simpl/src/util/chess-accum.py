# Quick script to compile CHESS results for all benchmarks

import os
import sys
import copy
import subprocess
import shutil
import pickle
import time 


def main():
    
    tr = os.environ.get('THRILLE_ROOT')
    if len(sys.argv) < 2:
        print "Usage: python chess-accum.py [exp dir name]"
        sys.exit()
    exp_name = sys.argv[1]
    assert tr is not None
    simpl_dir = os.path.join(tr, "benchmarks", "simpl")
    assert os.path.exists(simpl_dir)

    data = {}
    in_flight = {}

    for dir in os.listdir(simpl_dir):
        exp_path = os.path.join(simpl_dir, dir, \
                "exp", exp_name)
        result_path = os.path.join(exp_path, "chess.results")
        if not os.path.exists(result_path):
            if os.path.exists(exp_path):
                log_path = os.path.join(exp_path, "exp.log")
                if os.path.exists(log_path):
                    fin = open(log_path, "r").readlines()
                    in_flight[dir] = fin[-1].strip()
            continue
        fin = open(result_path, "r").readlines()
        data[dir] = fin

    print "Still working:"
    for x in in_flight:
        print x, ":", in_flight[x]
    
    print
    print "Done:"
    for x in data:
        print x, ":" 
        for y in data[x]:
            print "\t", y,



main()
