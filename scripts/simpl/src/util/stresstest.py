# stress tests a binary

import sys
import os
import copy
import shutil
import pickle
import subprocess
import traceback
import time
import signal


def checkEnvironment():
    if len(sys.argv) < 4:
        print "usage: python stresstest.py [times to execute]", 
        print "[desired result] [binary under test] [binary flags]"
        print
        print "purpose: repeatedly executes the binary always checking",
        print "the output contains the desired result"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert int(sys.argv[1]) > 0, "Nonsensical execution time"
    assert os.path.exists(sys.argv[2]), "desired result does not exist"
    assert os.path.exists(sys.argv[3]), "binary does not exist"

def main():
    checkEnvironment()
    tr = os.environ.get('THRILLE_ROOT')
    times_to_execute = int(sys.argv[1])
    desired_result = open(sys.argv[2], "r").readlines()
    binary_file = sys.argv[3]
    binary_flags = sys.argv[4:]
   
    i = 0
    while i < times_to_execute:
        log = open("execution.log", "w")
        bin_path = os.path.abspath(binary_file)
        args = [bin_path] + binary_flags
        os.environ["LD_PRELOAD"] = os.path.join(tr, "bin", "libserializer.so")
        subprocess.call(args, stdout=log, stderr=log)
        del os.environ["LD_PRELOAD"]
        log.close()
        log_in = open("execution.log", "r").readlines()
        for x in desired_result:
            found = False
            for y in log_in:
                if x in y:
                    found = True
                    break
            if not found:
                print "Could not find:", x
                assert False

        raw_input("Done an iteration")

        os.remove("execution.log")
        i += 1
        print "Iteration", i, "done"
        

if __name__ == "__main__":
    main()


