import os
import sys
import shutil

def customizeMakefile(newthriller, name):
    output = []
    fin = open(os.path.join(newthriller, "Makefile"), "r").readlines()
    for x in fin:
        if "PY**THRILLERNAME" in x:
            str = "THRILLERNAME="
            str += name
            str += "\n"
            output.append(str)
        elif "PY**SOURCENAME" in x:
            str = "SRCS=lib"
            str += name
            str += ".cpp"
            str += " createhandler.cpp interpose.cpp\n"
            output.append(str)
        elif "PY**HEADERNAME" in x:
            str = "HEADERS=lib"
            str += name
            str += ".h tls"
            str += name + ".h\n"
            output.append(str)
        elif "PY**TARGET" in x:
            str = "TARGET=lib"
            str += name
            str += ".so\n"
            output.append(str)
        elif "PY**TEST" in x:
            str = "TESTFILES=lib" + name + ".cpp\n"
            output.append(str)
        else:
            output.append(x)

    fout = open(os.path.join(newthriller, "Makefile"), "w")
    for x in output:
        fout.write(x)
    fout.close()
        

def customizeLibHeader(newthriller, name):
    output = []
    target = os.path.join(newthriller, "lib" + name + ".h")
    fin = open(target, "r").readlines()
    handlerName = name.capitalize() + "Handler"
    for x in fin:
        y = x.replace("BlankHandler", handlerName) 
        y = y.replace("LIBBLANK", "LIB" + name.upper())
        y = y.replace("libblank", "lib" + name.lower())
        y = y.replace("Blank", name.capitalize())
        y = y.replace("tlsblank", "tls" + name)
        output.append(y)

    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()

def customizeLibBody(newthriller, name):
    output = []
    target = os.path.join(newthriller, "lib" + name + ".cpp")
    fin = open(target, "r").readlines()
    handlerName = name.capitalize() + "Handler"
    for x in fin:
        if "PY**INCLUDE" in x:
            str = "#include \"lib" + name + ".h\"\n"
            output.append(str)
        else:
            y = x.replace("BlankHandler", handlerName) 
            y = y.replace("TLSBlankData", "TLS" + name.capitalize() + "Data")
            output.append(y)

    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()

def customizeData(newthriller, name):
    output = []
    target = os.path.join(newthriller, "tls" + name + ".h")
    fin = open(target, "r").readlines()
    for x in fin:
        y = x.replace("BLANK", name.upper())
        y = y.replace("Blank", name.capitalize())
        output.append(y)
    
    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()

def customizeHandler(newthriller, name):
    output = []
    target = os.path.join(newthriller, "createhandler.cpp")
    fin = open(target, "r").readlines()
    for x in fin:
        y = x.replace("BLANK", name.upper())
        y = y.replace("Blank", name.capitalize())
        y = y.replace("blank", name)
        output.append(y)

    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()

def customizeInterpose(newthriller, name):
    output = []
    target = os.path.join(newthriller, "interpose.cpp")
    fin = open(target, "r").readlines()
    for x in fin:
        y = x.replace("BLANK", name.upper())
        y = y.replace("Blank", name.capitalize())
        y = y.replace("blank", name)
        output.append(y)

    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()

def customizeTest(newtest, name):
    output = []
    target = os.path.join(newtest, "testlib" + name + ".h")
    fin = open(target, "r").readlines()
    for x in fin:
        y = x.replace("BLANK", name.upper())
        y = y.replace("Blank", name.capitalize())
        y = y.replace("blank", name)
        output.append(y)

    fout = open(target, "w")
    for x in output:
        fout.write(x)
    fout.close()


def main():
    if len(sys.argv) != 2:
        print "Usage: python create_new_project.py [project name]"
        print
        print "Creates an empty thriller"
        sys.exit()


    thriller = sys.argv[1]
    thriller = thriller.lower()
    thrille_root = os.environ.get('THRILLE_ROOT')

    if thrille_root == None:
        print "Error: env var THRILLE_ROOT is not set correctly"
        sys.exit()


    check = os.listdir(os.path.join(thrille_root,  'src'))

    if thriller in check:
        print "Error: a thriller with that name already exists"
        sys.exit()

    newthriller = os.path.join(thrille_root, 'src', thriller)
    newtest = os.path.join(thrille_root, 'tests', thriller)

    os.mkdir(newthriller)
    os.mkdir(newtest)

    fstart = os.path.join(thrille_root, 'scripts', 'config', 'blankthriller')


    startMake = os.path.join(fstart, 'Makefile')
    endMake = os.path.join(newthriller, 'Makefile')

    startLibHeader = os.path.join(fstart, 'libblank.h')
    endLibHeader = os.path.join(newthriller, 'lib' + thriller + '.h')

    startLibBody = os.path.join(fstart, 'libblank.cpp')
    endLibBody = os.path.join(newthriller, 'lib' + thriller + '.cpp')

    startData = os.path.join(fstart, 'tlsblank.h')
    endData = os.path.join(newthriller, 'tls' + thriller + '.h')

    startTest = os.path.join(fstart, "testlibblank.h")
    endTest = os.path.join(newtest, "testlib" + thriller + ".h")

    startHandler = os.path.join(fstart, 'createhandler.cpp')
    endHandler = os.path.join(newthriller, 'createhandler.cpp')

    startInterpose = os.path.join(fstart, 'interpose.cpp')
    endInterpose = os.path.join(newthriller, 'interpose.cpp')

    shutil.copy(startMake, endMake)
    shutil.copy(startLibHeader, endLibHeader)
    shutil.copy(startLibBody, endLibBody)
    shutil.copy(startData, endData)
    shutil.copy(startTest, endTest)
    shutil.copy(startHandler, endHandler)
    shutil.copy(startInterpose, endInterpose)


    customizeMakefile(newthriller, thriller)
    print "Makefile done..."
    customizeLibHeader(newthriller, thriller)
    print "Library header done..."
    customizeLibBody(newthriller, thriller)
    print "Library body done..."
    customizeData(newthriller, thriller)
    print "Thread local storage done..."
    customizeHandler(newthriller, thriller)
    print "Create handler done..."
    customizeInterpose(newthriller, thriller)
    print "Custom function interposition done"
    customizeTest(newtest, thriller)
    print "Unit test done..."


main()
