# This python script will run a binary until a bug is found

import sys
import os
import copy
import shutil
import pickle
import subprocess
import traceback
import time
import signal
import multiexp

tr = os.environ.get('THRILLE_ROOT')
script_src = os.path.join(tr, "scripts", "simpl", "src")
sys.path.append(os.path.join(script_src, "common"))
import schedule 

def checkEnvironment():
    if len(sys.argv) < 2:
        print "usage: python findbug.py [binary] [binary flags]",
        print
        print "runs binary until a bug is found"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert os.path.exists(sys.argv[1]), "binary does not exist"
    assert os.path.exists("thrille-randomactive"), "Need race data"

def main():
    checkEnvironment()
    binary_file = os.path.abspath(sys.argv[1])
    binary_file = os.path.normpath(binary_file)
    tr = os.environ.get('THRILLE_ROOT')
    binary_libs = os.path.join(tr, "bin")
    error_trace , executions = multiexp.getNewErrorSchedule(binary_libs, \
            binary_file, sys.argv[2:])

    error_sched = schedule.Schedule(error_trace)
    print "ERROR (", error_sched.error,") Found after",
    print executions , "executions"




if __name__ == "__main__":
    main()


