## 
# corecodegen.py - generates the thrille core files automatically 
#       from a specification found on disk
# 
# format := {function_name:[return_type, param1, ...]}
#
# Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal matter>
#

import marshal 
import copy
import pickle
import os

class ParamNamer:
    def __init__(self, myprefix = "param"):
        self.prefix = myprefix
        self.currentNumber = 0
        self.currErr = 0
        self.givenNames = []
        self.strTypeArgs = ""
        self.strTypes = ""
        self.numParams = 0

    def getParameterName(self):
        name = self.prefix + str(self.currentNumber)
        self.currentNumber += 1
        self.givenNames.append(name)
        return name

    def getName(self):
        if len(self.givenNames) < 1:
            return None
        else:
            return self.givenNames.pop(0)

    def errNo(self):
        tmp = str(self.currErr)
        self.currErr += 1
        return tmp

    def softReset(self):
        self.currentNumber = 0
        self.givenNames = []
        self.numParams = 0

    def reset(self):
        self.softReset()
        self.currErr = 0

    def numberOfParams(self):
        return self.numParams

    def getStrArgs(self):
        ret_str = ""
        for x in self.givenNames:
            ret_str += x + ", "
        return ret_str[:-2]

    def processParams(self, paramlist):
        self.softReset()
        str = ""
        typestr = ""
        while len(paramlist) > 0:
            self.numParams += 1
            param = paramlist.pop(0)
            typestr += param
            param = param.replace("restrict", "")
            #for create, special param
            if "(*)" in param:
               tmpstrlist = param.split("(*)")
               assert len(tmpstrlist) == 2, "err 2"
               str += tmpstrlist[0] + "(* " + self.getParameterName() + ")"
               str += tmpstrlist[1]
            else:
                str += param + " " + self.getParameterName()
            if len(paramlist) > 0:
                str += ", " 
                typestr += ", "
        self.strTypeArgs = str
        self.strTypes = typestr


    def getStrTypes(self):
        return self.strTypes

    def getStrTypeArgs(self):
        return self.strTypeArgs 
 

def getPthreadAPI(root):
    targetdir = os.path.join(root, "scripts", "config")
    if "pthreaddict" not in os.listdir(targetdir):
        print "Error: pthread api dictionary not found"
        sys.exit()

    target = os.path.join(targetdir, "pthreaddict") 
    fin = open(target, "r")
    apidict = marshal.load(fin)
    fin.close()
    return apidict 

def genOrigStaticPublic(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        str = "        static "
        str += returntype + " " + func + "("
        namer.processParams(paramlist)
        str += namer.getStrTypes() + ");\n"
        outfile.append(str)


def genOrigStaticPrivate(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        str = "        static "
        str += returntype + " (* volatile _" + func + ") ("
        namer.processParams(paramlist)
        str += namer.getStrTypes() + ");\n"
        outfile.append(str)

def generateOriginalsHeader(dict, root):
    target = os.path.join(root, "scripts", "config", "blankcore")
    target = os.path.join(target, "Originals.h")
    fin = open(target, "r")
    blankfile = fin.readlines()
    fin.close()
    
    outfile = [] 

    for x in blankfile:
        if "PYTHON_STATIC_PUBLIC" in x:
            genOrigStaticPublic(dict, outfile)
        elif "PYTHON_STATIC_PRIVATE" in x:
            genOrigStaticPrivate(dict, outfile)
        else:
            outfile.append(x)

    target = os.path.join(root, "src", "thrille-core")
    target = os.path.join(target, "Originals.h")
    fout = open(target, "w")
    for x in outfile:
        fout.write(x)
    fout.close()
    print "Originals Header done..."

def genOrigInitNull(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        str = returntype + " (* volatile Originals::_" + func + ") ("
        namer.processParams(paramlist)
        str += namer.getStrTypes() + ") = NULL;\n"
        outfile.append(str)

def genOrigInit(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        namer.processParams(paramlist)
        str = "    init_original(" + func + ", "
        str += returntype + " (* volatile) ("
        str += namer.getStrTypes() + "));\n"
        outfile.append(str)

def genOrigImpl(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        str = mydict[func].pop(0)  + " Originals::" + func + "("
        paramlist = mydict[func]
        namer.processParams(paramlist)
        str += namer.getStrTypeArgs()
        str += ") {\n    safe_assert(_" + func + " != NULL" 
        str += ");\n    "
        
        #special (no) return for exit
        if func != "pthread_exit":
            str += "return _" 
        else:
            str += "_"
        str += func + "(" + namer.getStrArgs() + ");\n}\n\n"
        outfile.append(str)
        namer.softReset()

def generateOriginals(dict, root):
    target = os.path.join(root, "scripts", "config", "blankcore")
    target = os.path.join(target, "Originals.cpp")
    fin = open(target, "r")
    blankfile = fin.readlines()
    fin.close()
    
    outfile = [] 

    for x in blankfile:
        if "PYTHON_INIT_NULL" in x:
            genOrigInitNull(dict, outfile)
        elif "PYTHON_INIT_ORIGINALS" in x:
            genOrigInit(dict, outfile)
        elif "PYTHON_ORIG_IMPL" in x:
            genOrigImpl(dict, outfile)
        else:
            outfile.append(x)

    target = os.path.join(root, "src", "thrille-core")
    target = os.path.join(target, "Originals.cpp")
    fout = open(target, "w")
    for x in outfile:
        fout.write(x)
    fout.close()
    print "Originals Cpp done..."

def getHandleNames(func):
    myfuncwords = func.split("_")
    if myfuncwords[0] == "pthread":
        myfuncwords.pop(0)
    basestr = ""
    for x in myfuncwords:
        basestr += x.capitalize()
    return ("Before" + basestr, "Simulate" + basestr, "After" + basestr)

def genLibProtHandles(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        handlenames = getHandleNames(func)
        assert len(handlenames) == 3, "err 3"
        for hname in handlenames:
            str = "        virtual "
            paramlist = copy.deepcopy(mydict[func])
            returntype = paramlist.pop(0)
            namer.processParams(paramlist)
            if "Before" in hname:
                if namer.numberOfParams() > 0:
                    str += "bool " +  hname + "(void * ret_addr, "
                else:
                    str += "bool " +  hname + "(void * ret_addr"
            elif "Simulate" in hname:
                if namer.numberOfParams() > 0:
                    str += returntype + " " +  hname + "(void * ret_addr, "
                else:
                    str += returntype + " " +  hname + "(void * ret_addr"
            elif "After" in hname:
                if returntype == "void":
                    if namer.numberOfParams() > 0:
                        str += "void " + hname + "(void * ret_addr, "
                    else:
                        str += "void " + hname + "(void * ret_addr"
                else:
                    if namer.numberOfParams() > 0:
                        str += "void " +  hname + "(void * ret_addr, " 
                        str +=  returntype + ", "
                    else:
                        str += "void " +  hname + "(void * ret_addr, " 
                        str += returntype 
            else:
                assert False, "err 4"
            if func == "pthread_create" and "After" not in hname:
                str += namer.getStrTypes() + ", ThreadInfo*&);\n"
            else: 
                str += namer.getStrTypes() + ");\n"
            outfile.append(str)
        outfile.append("\n")

def genLibPrivImpl(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        str = "        "
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        namer.processParams(paramlist)
        thrillename = func.replace("pthread", "thrille")
        if namer.numberOfParams() > 0:
            str += returntype + " " +  thrillename + " (void * ret_addr, "
        else:
            str += returntype + " " +  thrillename + " (void * ret_addr"
        str += namer.getStrTypes() + ");\n"
        outfile.append(str)

def genLibFriendDecs(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        namer.processParams(paramlist)
        str = "        friend " + returntype + " " + func + "("
        str += namer.getStrTypes() +  ");\n"
        outfile.append(str)

def generateLibHeader(dict, root):
    target = os.path.join(root, "scripts", "config", "blankcore")
    target = os.path.join(target, "libpth.h")
    fin = open(target, "r")
    blankfile = fin.readlines()
    fin.close()
    
    outfile = [] 

    for x in blankfile:
        if "PYTHON_PROTECTED_HANDLES" in x:
            genLibProtHandles(dict, outfile)
        elif "PYTHON_PRIVATE_IMPL" in x:
            genLibPrivImpl(dict, outfile)
        elif "PYTHON_FRIEND_DECS" in x:
            genLibFriendDecs(dict, outfile)
        else:
            outfile.append(x)
    
    target = os.path.join(root, "src", "thrille-core")
    target = os.path.join(target, "libpth.h")
    fout = open(target, "w")
    for x in outfile:
        fout.write(x)
    fout.close()
    print "Libpth Header done..."

def genLibHandleImpl(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        if "pthread_create" in func:
            continue

        handlenames = getHandleNames(func)
        assert len(handlenames) == 3, "err 4"
 
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        str = returntype + " Handler::" + func.replace("pthread", "thrille")
        namer.processParams(paramlist)
        if namer.numberOfParams() > 0:
            str += "(void * ret_addr, " + namer.getStrTypeArgs()
        else:
            str += "(void * ret_addr" + namer.getStrTypeArgs()

        str += ") {\n    if(insideInst) {\n        printf(\""
        str += "ThrilleErr: In Instrumentation in " + func + "\\n\");\n"
        str += "        _Exit(3);\n    }\n"
        if returntype != "void":
            str += "    " + returntype + " ret_val;\n" 
        str += "    ++insideInst;\n\n"
        str += "    bool call_original = " + handlenames[0] 
        if namer.numberOfParams() > 0:
            str += "(ret_addr, "
        else:
            str += "(ret_addr"
        str += namer.getStrArgs() + ");\n\n"
        if func == "pthread_exit":
            str += "    --insideInst;\n"
        str += "    if (call_original)\n"
        if returntype != "void":
            str += "        ret_val = Originals::" + func + "(" 
            str += namer.getStrArgs() + ");\n    else\n"
            str += "        ret_val = " + handlenames[1] 
            if namer.numberOfParams() > 0:
                str += "(ret_addr, "
            else:
                str += "(ret_addr"
            str += namer.getStrArgs() + ");\n\n"
            str += "    " + handlenames[2] 
            if namer.numberOfParams() > 0:
                str +=  "(ret_addr, ret_val, " + namer.getStrArgs()
            else:
                str +=  "(ret_addr, ret_val"
            str += ");\n    --insideInst;\n    return ret_val; \n}\n\n"
        else:
            str += "        Originals::" + func + "(" 
            str += namer.getStrArgs() + ");\n    else\n"
            if namer.numberOfParams() > 0:
                str += "        " + handlenames[1] + "(ret_addr, "
            else:
                str += "        " + handlenames[1] + "(ret_addr"
            str += namer.getStrArgs() + ");\n\n"
            if namer.numberOfParams() > 0:
                str += "    " + handlenames[2] + "(ret_addr, " 
            else:
                str += "    " + handlenames[2] + "(ret_addr" 
            str += namer.getStrArgs()
            str += ");\n    --insideInst;\n}\n\n"
        outfile.append(str)
        namer.softReset()

def genLibEmptyHooks(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        if "pthread_create" in func:
            continue
        handlenames = getHandleNames(func)
        assert len(handlenames) == 3, "err 5"
        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        namer.processParams(paramlist)
        for hname in handlenames:
            str = ""
            if "Before" in hname:
                if namer.numberOfParams() > 0:
                    str += "bool Handler::" +  hname + "(void * ret_addr, "
                else:
                    str += "bool Handler::" +  hname + "(void * ret_addr"
            elif "Simulate" in hname:
                str += returntype + " Handler::" +  hname
                if namer.numberOfParams() > 0:
                    str += "(void * ret_addr, "
                else:
                    str += "(void * ret_addr"
            elif "After" in hname:
                if returntype == "void":
                    str += "void Handler::" +  hname
                    if namer.numberOfParams() > 0:
                        str += "(void * ret_addr, "
                    else:
                        str += "(void * ret_addr"
                else:
                    if namer.numberOfParams() > 0:
                        str += "void Handler::" +  hname
                        str += "(void * ret_addr, " + returntype + " ret_val, "
                    else:
                        str += "void Handler::" 
                        str += hname + "(void * ret_addr, " + returntype 
                        str += " ret_val"
            else:
                assert False, "err 6"
            str += namer.getStrTypeArgs()
            str +=  ") {"
            if "Before" in hname:
                str += "\n    if (safe_mode) {"
                str += "\n        bool functionIsImplemented = false;"
                str += "\n        safe_assert(functionIsImplemented);"
                str += "\n    }"
                str += "\n    return true;"
                str += "\n}\n"
            elif "Simulate" in hname:
                if returntype == "void":
                    str += "return;}\n" 
                elif returntype == "int":
                    str += "return 0;}\n"
                elif returntype == "void *":
                    str += "return (void *) 0;}\n"
                elif returntype == "pthread_t":
                    str += "pthread_t s;\n return s;}\n"
                elif returntype == "unsigned int":
                    str += "return 0;}\n"
                else:
                    print "PYERROR 8"
                    print returntype
                    sys.exit()
            elif "After" in hname:
                str += " }\n" 
            else:
                assert False, "err 7"
            outfile.append(str)
        outfile.append("\n")


def genLibInterposImpl(dict, outfile):
    mydict = copy.deepcopy(dict)
    namer = ParamNamer()
    for func in dict:
        if "pthread_create" in func:
            continue

        paramlist = mydict[func]
        returntype = paramlist.pop(0)
        str = returntype + " " + func + "("
        namer.processParams(paramlist)
        str += namer.getStrTypeArgs() + ") "
        if "pthread_self" in func or "pthread_getconcurrency" in func:
            str += "__THROW "
        if "sched_yield" in func:
            str += "throw () "
        str += "{\n"
        str += "    if (! Initializer::is_done()) {\n"
        str += "        bool success = Initializer::run();\n"
        str += "        if (! success) {\n"
        str += "            printf(\"" + func + " initializer fail\\n\");\n"
        str += "            throw 13;\n"
        str += "        }\n    }\n\n"
        str += "    void * ret_addr = __builtin_return_address(0);\n"
       #special (no) return for exit
        if func != "pthread_exit":
            str += "    return pHandler->" + func.replace("pthread", "thrille")
        else:
            str += "    pHandler->" + func.replace("pthread", "thrille")
        if namer.numberOfParams() > 0:
            str += "(ret_addr, " + namer.getStrArgs() + ");\n"
        else:
            str += "(ret_addr);\n"
        if func == "pthread_exit":
            str += "    printf(\"ThrilleErr: problem with pthread_exit\\n\");"
            str += "\n    throw 59;\n"
        str += "}\n\n"
        outfile.append(str)
        namer.softReset()
 
def generateLib(dict, root):
    target = os.path.join(root, "scripts", "config", "blankcore")
    target = os.path.join(target, "libpth.cpp")
    fin = open(target, "r")
    blankfile = fin.readlines()
    fin.close()
    
    outfile = [] 

    for x in blankfile:
        if "PYTHON_HANDLER_IMPL" in x:
            genLibHandleImpl(dict, outfile)
        elif "PYTHON_EMPTY_HOOKS" in x:
            genLibEmptyHooks(dict, outfile)
        elif "PYTHON_INTERPOS_IMPL" in x:
            genLibInterposImpl(dict, outfile)
        else:
            outfile.append(x)

    target = os.path.join(root, "src", "thrille-core")
    target = os.path.join(target, "libpth.cpp")
    fout = open(target, "w")
    for x in outfile:
        fout.write(x)
    fout.close()
    print "Libpth Cpp done..."

def cleanseDict(dict):
    try:
        del dict["pthread_cleanup_push"]
    except:
        pass
    try:
        del dict["pthread_cleanup_pop"]
    except:
        pass

    for func in dict:
        if len(dict[func]) == 2:
            if "void" in dict[func][1] and "*" not in dict[func][1]:
                dict[func].pop()

def main():
    thrille_root = os.environ.get('THRILLE_ROOT')
    
    if thrille_root == None:
        print "Error: env var THRILLE_ROOT is not set correctly"
        sys.exit()

    dict = getPthreadAPI(thrille_root)
    cleanseDict(dict)
    generateOriginalsHeader(dict, thrille_root)
    generateOriginals(dict, thrille_root)
    generateLibHeader(dict, thrille_root)
    generateLib(dict, thrille_root)


main()
