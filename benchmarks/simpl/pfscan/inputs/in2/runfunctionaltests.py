# runfunctionaltests.py
#
# Author: Nick Jalbert (jalbert@eecs)
#
# Harness for running functional tests on thrillers
# Automatically checks results against saved results (ensures that saved
#       results are a subset of test results -- gives flexibility in 
#       specifiying what is important to test for)
# Can generate results files
#
# Expects the directory structure to be in the following manner:
#       $(THRILLE_ROOT)/tests/[thriller_name]/test
#
# in the test dir:      -a set of .c or .cpp tests
#                       -[optional] each [name].c and [name].cpp can
#                               have these associated scripts which
#                               will set up and teardown the environment
#                                       -setup[name].sh
#                                       -teardown[name].sh
#                       -[optional] each test suite can have a 
#                               set of scripts which will be run
#                               at the beginning and end of the suite:
#                                       -setupall.sh
#                                       -teardownall.sh
#                       -a file "info" which has the thriller binary name
#                               format:
#                                       THRILLER_NAME:[binary name]
#                       -a directory 'results' which contains the
#                               saved results to which we compare
#                               our test runs
#                       

import os
import sys
import subprocess
import time

# Inputs:       tr - thrille root  (abs path)
#               td - test dir (abs path)
#               testin - test source to compile 
#               testout - final compile target
#
#Outputs:       None (compile errors will be recorded in
#                     compile.log in test dir)
def llvmCompile(tr, td, testin, testout):
    assert os.path.exists(td)
    cwd = os.getcwd()
    os.chdir(td)
    compiler = ""
    if ".cpp" in testin:
        compiler = "llvm-g++"
    else:
        assert ".c" in testin
        compiler = "llvm-gcc"

    fout = open("compile.log", "w")
    
    args = [compiler, "-c", "-emit-llvm", "-g", testin, "-o", "tmp.o"]
    exit = subprocess.call(args, stdout = fout, stderr = fout)
    assert exit == 0
    assert os.path.exists(os.path.join(td, "tmp.o"))
    bin  = os.path.join(tr, "bin")
    opt = os.path.join(bin, "opt")
    llvmpass = os.path.join(bin, "LLVMLoadStore.so")
    assert os.path.exists(opt)
    assert os.path.exists(llvmpass)
    args = [opt, "-load", llvmpass, "-loadstore", "tmp.o", "-f", "-o", "tmppass.o"]
    exit = subprocess.call(args, stdout = fout, stderr = fout)
    os.remove("tmp.o")
    os.remove("iiddump")
    assert exit == 0
    llc = os.path.join(bin, "llc")
    assert os.path.exists(llc)
    assert os.path.exists(os.path.join(td, "tmppass.o"))
    args = [llc, "-f", "tmppass.o"]
    exit = subprocess.call(args, stdout = fout, stderr = fout)
    os.remove("tmppass.o")
    assert exit == 0
    assert os.path.exists(os.path.join(td, "tmppass.o.s"))
    link = "-L" + bin
    args = [compiler, "tmppass.o.s", "-o", testout, link, \
            "-ldummy", "-lpthread"]
    exit = subprocess.call(args, stdout = fout, stderr = fout)
    os.remove("tmppass.o.s")
    assert exit == 0
    assert os.path.exists(testout)
    os.remove("compile.log")
    fout.close()
    os.chdir(cwd)

# Inputs:       test_exec - test executable (abs path)
#               thriller_bin - the library to preload (abs path)
#               outfile - if None, output will be a tmp and deleted
#                       otherwise output will be saved to outfile
#
# Outputs:      list of everything printed by running the test
def runTest(test_exec, thriller_bin, outfile = None):
    cwd = os.getcwd()
    td, tmp = os.path.split(test_exec)
    assert os.path.exists(td)
    os.chdir(td)
    clean_up = False
    if outfile == None:
        outfile = "pyfunctest.tmp.out"
        clean_up = True
    os.environ['LD_PRELOAD'] = thriller_bin
    fout = open(outfile , "w")
    p = subprocess.Popen(test_exec, stdout = fout, stderr = fout) 
    starttime = time.time()
    newtime = time.time()
    while p.returncode == None and ((newtime - starttime) < 5):
        p.poll()
        time.sleep(.05)
        newtime = time.time()
    if (newtime > starttime + 5):
        printFailure(test_exec, ["Potential Deadlock"], ["No Deadlock"])
        del os.environ['LD_PRELOAD']
        os.system("kill " + str(p.pid))
        return []
    fout.flush()
    fout.close()
    fout = open(outfile, "r")
    results = fout.readlines()
    fout.close()
    if clean_up:
        os.remove(outfile)
    del os.environ['LD_PRELOAD']
    os.chdir(cwd)
    return results


# Inputs:       results_dir - path to where results are stored
#               test - test name
#
# Outputs:      list of everything in expected results
def getExpectedResults(results_dir, test):
    result_file = os.path.join(results_dir, test + ".out")
    assert os.path.exists(result_file)
    fin = open(result_file, "r")
    results = fin.readlines()
    fin.close()
    return results

# Inputs:       superlist - list that should contain everything 
#                       in sublist (and potentially more)
#               sublist - list of what should be in superlist
#
# Outputs:      String of item in sublist but not in superlist
#               or None if no item is found
def getDifference(superlist, sublist):
    for x in sublist:
        tmp_check = x.strip()
        found = False
        for y in superlist:
            if tmp_check in y:
                found = True
                break
        if not found:
            return tmp_check
    return None




# Inputs:       test_result - list of what was printed when test was run
#               expected_result - list of what we want to see for a pass
#
# Outputs:      Bool, whether test passes or not
def checkResults(test_result, expected_result):
    item = getDifference(test_result, expected_result)
    if item is None:
        return True
    return False

# Inputs:               test - name of the test suite
#                       test_results - list of results we got
#                       expected_results - what we expected to see
#
# Outputs:              None
def printFailure(test, test_result, expected_result):
    print "\tERROR: test", test, "failed"
    print "\tWe got the following results:"
    for x in test_result:
        print "\t\t", x,
    print
    print "\tWe expected to see the following:"
    for x in expected_result:
        print "\t\t", x,
    print
    print "\tMissing:"
    print "\t\t", getDifference(test_result, expected_result)
    print

        


# Inputs:               tr - thrille root (abs path) 
#                       td - test dir  (abs path)
#
# Outputs:              thriller_bin - thriller under test (abs path)
#                       results_dir - place were expected results reside 
def getTestSuiteInfo(tr, td):
    assert os.path.exists(os.path.join(td, "info")), "Need info file"
    info = open(os.path.join(td,"info"), "r").readlines()
    assert len(info) == 1
    thriller_bin = info[0].strip()
    thriller_bin = thriller_bin.split(":")[1]
    thriller_bin = os.path.join(tr, "bin", thriller_bin)
    assert os.path.exists(thriller_bin)
    results_dir = os.path.join(td, "results")
    assert os.path.isdir(results_dir)
    return thriller_bin, results_dir


# Inputs:       test - name of test file
#
# Outputs:      Name of test without any pesky extensions
def getTestName(test):
    if ".cpp" in test:
        test = test[:-4]
    elif ".c" in test:
         test = test[:-2]
    else:
        print file
        assert False, "Unrecognized test type"
    return test

# Inputs:       path - script path(abs path)
#               script - name of script to run
#
# Outputs:      None   
def runScript(path, script):
    assert os.path.isdir(path)
    cwd = os.getcwd()
    os.chdir(path)
    scriptpath = os.path.join(path, script)
    assert os.path.exists(scriptpath)
    fout = open(script + ".log", "w")
    args = [scriptpath]
    exit = subprocess.call(args, stdout = fout, stderr = fout)
    fout.close()
    assert exit == 0
    os.remove(script + ".log")
    os.chdir(cwd)

# Inputs:       td - test dir path(abs path)
#               name - name of script to run
#
# Outputs:      True if script was found and run, else False 
def runSetupScript(td, name):
    assert os.path.isdir(td)
    script_dir = os.path.join(td, "scripts")
    assert os.path.isdir(script_dir)
    if "setup" + name + ".sh" in os.listdir(script_dir):
        runScript(script_dir, "setup" + name + ".sh")
        return True
    return False

# Inputs:       td - test dir path(abs path)
#               name - name of script to run
#
# Outputs:      True if script was found and run, else False 
def runTeardownScript(td, name):
    assert os.path.isdir(td)
    script_dir = os.path.join(td, "scripts")
    assert os.path.isdir(script_dir)
    if "teardown" + name + ".sh" in os.listdir(script_dir):
        runScript(script_dir, "teardown" + name + ".sh")
        return True
    return False

# Inputs:       tr - thrille root (abs path)
#               td - test dir  (abs path)
#
# Outputs:       None
def executeTestSuite(tr, td):
    thriller_bin, results_dir = getTestSuiteInfo(tr, td)
    test_dir_list = os.listdir(td)
    runSetupScript(td, "all")
    fail_list = []
    tests_passed = 0
    tests_failed = 0
    for x in test_dir_list:
        if (".cpp" not in x) and (".c" not in x):
            continue
        test = getTestName(x) 
        if (test + ".out") not in os.listdir(results_dir):
            print "\tWARNING:",
            print "no result file for test", test 
            print "\t",test, "will be skipped..."
            print
            continue
        runSetupScript(td, test)
        llvmCompile(tr, td, x, test)
        test_executable = os.path.join(td, test)
        assert os.path.exists(test_executable)
        test_result = runTest(test_executable, thriller_bin)
        expected_result = getExpectedResults(results_dir, test)
        if checkResults(test_result, expected_result):
            tests_passed += 1
        else:
            tests_failed += 1
            fail_list.append(test)
            printFailure(test, test_result, expected_result)
        runTeardownScript(td, test)

    runTeardownScript(td, "all")
    print "\tTests Passed:", tests_passed
    print "\tTests Failed:", tests_failed
    for x in fail_list:
        print "\t\t", x
        
# Inputs:       tr - thrille root (abs path)
#
# Outputs:       list of test suites which pass the sanity checks and
#                       can be run by this script
def getTestSuites(tr):
    test_list = []
    testdir = os.path.join(tr, "tests")
    for dir in os.listdir(testdir):
        if ".svn" in dir:
            continue
        thriller_tests= os.path.join(testdir, dir, "test")
        if not os.path.isdir(thriller_tests):
            continue
        if len(os.listdir(thriller_tests)) < 3:
            continue
        if "info" not in os.listdir(thriller_tests):
            continue
        test_results = os.path.join(thriller_tests, "results")
        if not os.path.isdir(test_results):
            continue
        if len(os.listdir(test_results)) < 1:
            continue
        test_list.append(dir)
    return test_list

# Inputs:       tr - thrille root (abs path)
#               td - test dir (abs path)
#
# Outputs:      None (generates a result file for each
#                       test without one)
#
def generateResultFiles(tr, td):
    thriller_bin, results_dir = getTestSuiteInfo(tr, td)
    runSetupScript(td, "all")
    tests_generated = 0
    for x in os.listdir(td):
        if (".cpp" not in x) and (".c" not in x):
            continue
        test = getTestName(x) 
        if (test + ".out") in os.listdir(results_dir):
            print "\tResult file for", test, "exists"
            print "\tNew result file will NOT be generated"
            print 
            continue
        runSetupScript(td, test)
        llvmCompile(tr, td, x, test)
        test_executable = os.path.join(td, test)
        assert os.path.exists(test_executable)
        result_out = os.path.join(results_dir, test + ".out")
        test_result = runTest(test_executable, thriller_bin, result_out)
        assert len(test_result) > 0
        tests_generated += 1
        runTeardownScript(td, test)
    
    runTeardownScript(td, "all") 
    print "\tTests Generated:", tests_generated


def main():
    tr = os.environ.get('THRILLE_ROOT');
    assert tr != None
    if len(sys.argv) < 2:
        print "usage: python runfunctionaltests.py [option]"
        print "purpose: runs functional tests for thrillers"
        print
        print "[options]:"
        print "\t-a : runs all tests"
        print "\t-ls : lists possible tests"
        print "\t-t [name] : runs test for thriller [name]"
        print "\t-g [name] : generate result directory for thriller [name]"
        print
        sys.exit(0)

    if "-a" in sys.argv[1]:
        assert len(sys.argv) == 2, "Malformed commandline parameters"
        test_list = getTestSuites(tr)
        for test in test_list:
            print
            print "**Running Suite", test, "**"
            test_path = os.path.join(tr, "tests", test, "test")
            executeTestSuite(tr, test_path)
            print "**Finished Suite", test, "**"
            print


    if "-ls" in sys.argv[1]:
        assert len(sys.argv) == 2, "Malformed commandline parameters"
        test_list = getTestSuites(tr)
        print "Test Suites:"
        for x in test_list:
            print "\t", x
        sys.exit(0)


    if "-t" in sys.argv[1]:
        assert len(sys.argv) == 3, "Malformed commandline parameters"
        test = sys.argv[2]
        assert test in getTestSuites(tr), "Unknown test suite"
        test_path = os.path.join(tr, "tests", test, "test")
        print
        print "**Running Suite", test, "**"
        executeTestSuite(tr, test_path)
        print "**Finished Suite", test, "**"
        print

    if "-g" in sys.argv[1]:
        assert len(sys.argv) == 3, "Malformed commandline parameters"
        test = sys.argv[2]
        test_path = os.path.join(tr, "tests", test, "test")
        assert os.path.isdir(test_path), "Not a valid thriller"
        print "**Generating Result Files For", test,"**"
        generateResultFiles(tr, test_path)
        print "**Done Generating Result Files For", test,"**"
        print


main()
