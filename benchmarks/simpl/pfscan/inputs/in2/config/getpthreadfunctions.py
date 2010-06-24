##
# getpthreadfunctions.py - outputs the pthread man page to mapthread.txt
# parses the latter, creates a dictionary with pairs 
# (functionname, list of function args where last element is result type)
# marshals dictionary to pthreaddict file
#
# Author - Christos Stergiou (chster@eecs.berkeley.edu)
#

import os,re,marshal

os.system('man pthread | col -b > manpthread.txt')
filemp = open('manpthread.txt')
filedict = open('pthreaddict','w')
try:
    pfuncs = dict()
    previousmatch = False
    funcargtypesstr = ''
    funcname = ''
    funcrettype = ''
    for line in filemp:
        line = line.rstrip('\n')
        funcargtypeslist = []
        if previousmatch:
            previousmatch = False
            funcargtypesstr = funcargtypesstr + ' ' + line.strip()[0:-2]
        else:
            #matchobj = re.search('[\t ]*[([a-zA-Z0-9_]+)[\t ]+([a-zA-Z0-9_]+)\(([a-z]+.*$)', line)
            matchobj = re.search('[\t ]*([a-zA-Z0-9_]+( \*)?)[\t ]*([a-zA-Z0-9_]+)\(([a-z]+.*$)', line)
            if matchobj:
                funcname = matchobj.group(3)
                funcrettype = matchobj.group(1)
                funcargtypesstr = matchobj.group(4);
                if not re.search(';$', matchobj.group(4)):
                    # function arguments continue to next line
                    previousmatch = True
                    continue
                else:
                    # remove ');' from end of line 
                    funcargtypesstr = funcargtypesstr[0:-2]
        if matchobj or previousmatch:
            funcargtypeslist = re.split(', ', funcargtypesstr)
            funcargtypeslist.reverse()
            funcargtypeslist.append(funcrettype)
            funcargtypeslist.reverse()
            print funcname,"->",funcargtypeslist
            pfuncs[funcname] = funcargtypeslist

finally:
    marshal.dump(pfuncs,filedict)
    filemp.close()
    filedict.close()



