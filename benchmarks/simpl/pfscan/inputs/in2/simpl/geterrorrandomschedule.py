import sys
import os
import copy
import shutil
import subprocess
tr = os.environ.get('THRILLE_ROOT')
if tr != None:
    sys.path.append(os.path.join(tr, "scripts/simpl/blockremoval"))
    import blockremoval
    sys.path.append(os.path.join(tr, "scripts/simpl/preemptremoval"))
    import preemptremoval
    sys.path.append(os.path.join(tr, "scripts/simpl/all"))
    import noniterativeAA
    import noniterativeBB
    import multiexp

def checkForError():
    assert os.path.exists("py.tmp.out")
    progOutput = open("py.tmp.out", "r").readlines()
    endingFound = False
    for x in progOutput:
        print x,
        if "Segmentation" in x or "assert" in x or "ERROR" in x:
            return True
        if "Thrille:Ending" in x:
            endingFound = True
    if endingFound:
        return False 
    else:
        return True 

def getError(binary_file, thrille_root):
    assert os.path.exists(os.path.join(thrille_root, "bin/liblockrace.so"))
    assert os.path.exists(os.path.join(thrille_root, "bin/librandact.so"))
    binarydir, bin = os.path.split(binary_file)
    curr_dir = os.getcwd()
    if binarydir != '':
        os.chdir(binarydir)
    binflags = []
    for x in sys.argv[2:]:
        binflags.append(x)
    binflags.insert(0, "./" +bin)
    #print multiexp.executePreload(thrille_root, "liblockrace.so",\
    #        binary_file, binflags)
    assert os.path.exists("./thrille-randomactive")
    multiexp.clearOldThrilleSchedule()
    exit_status = 0
    sched = []
    enabled = []
    error = ""
    count = 0;
    while True:
        count += 1
        multiexp.clearOldThrilleSchedule()
        print "***ITERATION NUMBER", count
        os.environ['LD_PRELOAD'] = os.path.join(tr, "bin", "librandomschedule.so")
        fout = open("py.tmp.out", "w")
        print "calling:", binflags
        exit_status = subprocess.call(binflags, stdout=fout, stderr=fout)
        fout.close()
        del os.environ['LD_PRELOAD']
        errorFound = checkForError()
        if errorFound:
            print "ERROR FOUND AFTER", count
            shutil.copy(os.path.join(binarydir, "my-schedule"), \
                    os.path.join(binarydir, "py-error-schedule"))
            os.remove("py.tmp.out")
            return


def checkEnvironment():
    if len(sys.argv) < 2:
        print "usage: python geterror.py ",
        print "[binary under test] [binary flags]"
        print
        print "purpose: runs a combination of race detection and",
        print "random active testing until an failing execution is detected"
        print 
        print "Output: on termination, my-schedule in the binary directory",
        print "will contain the offending schedule"
        print
        sys.exit(1)

    assert os.environ.get('THRILLE_ROOT')!= None, \
            "Thrille root environment variable not defined"
    assert os.path.exists(sys.argv[1]), "binary does not exist"


def subprocessTest():
    binarydir, bin = os.path.split(sys.argv[1])
    insert = []
    insert.append(sys.argv[1])
    for x in sys.argv[2:]:
        insert.append(x)
    print "Passing", insert
    fout = open("py.tmp.out", "w")
    os.environ['LD_PRELOAD'] =  \
            os.path.join(tr, "bin", "libserializer.so")
    ret = subprocess.call(insert, stdout=fout, stderr=fout)
    print ret
    del os.environ['LD_PRELOAD']
    print "here"



def main():
    checkEnvironment()
    tr = os.environ.get('THRILLE_ROOT')
    getError(sys.argv[1], tr)




if __name__ == "__main__":
    main()

