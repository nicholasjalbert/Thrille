# Purpose: this file will parse the ouput of a  run of the relaxed scheduler
# and turn the context switch information into a human readable form for 
# debugging purposes
#
# Author - Nick Jalbert (jalbert@eecs.berkeley.edu)
#
# <Legal Matter>

import os
import sys
import subprocess


def addr2lineQuery(binary, addr):
    line_query = ["addr2line", "-e", binary, addr]
    assert not os.path.exists("tmpaddrquery.out")
    fout = open("tmpaddrquery.out", "w")
    subprocess.call(line_query, stdout=fout, stderr=fout)
    assert os.path.exists("tmpaddrquery.out")
    line_result = open("tmpaddrquery.out", "r").readlines()
    assert len(line_result) == 1
    line_info = line_result[0].strip()
    os.remove("tmpaddrquery.out")
    assert not os.path.exists("tmpaddrquery.out")
    return line_info[line_info.rfind("/"):].strip()


def main():
    if len(sys.argv) != 3:
        print
        print "usage: python generateScheduleInfo.py [binary]",
        print " [output of relaxed serializer]"
        print 
        print "purpose: attempts to parse the output of the relaxed",
        print "serializer and generate human readable information about",
        print "where context switches occurred in an execution.  Outputs ",
        print "to switch_summary.log"
        print
        sys.exit(0)

    assert os.path.exists(sys.argv[1])
    assert os.path.exists(sys.argv[2])
    binary = sys.argv[1]
    trace = sys.argv[2]
    readable_trace = []
    switch_list = []
    fin = open(trace, "r").readlines()
    for x in fin:
        if "Preemption(" in x:
            switch_list.append(x.strip())
        elif "NonPreemptive" in x:
            switch_list.append(x.strip())
        if "0x" in x:
            addr = x.find("0x")
            q = x[addr:]
            closeBracket = False 
            if ")" in q:
                closeBracket = True
            q.strip(")")
            debuginfo = addr2lineQuery(binary, q)
            if "??" in debuginfo:
                readable_trace.append(x)
            else:
                if closeBracket:
                    readable_trace.append(x[:addr] + debuginfo + ")\n")
                else:
                    readable_trace.append(x[:addr] + debuginfo + "\n")
        else:
            readable_trace.append(x)


    outfile = open("readable_trace.log", "w")
    for x in readable_trace:
        outfile.write(x)
    outfile.close()



    outfile = open("switch_summary.log", "w")

    while len(switch_list) > 0:
        item = switch_list.pop(0)
        item_list = item.split(":")
        assert len(item_list) == 4
        string = ""
        if "Preemption" in item_list[0]:
            string += "Preemptive Context Switch from thread "
        else:
            assert "NonPreemptive" in item_list[0]
            string += "Non-preemptive Context Switch from thread "

        string += item_list[1] + " to thread " + item_list[2] + " at "

        line_info = addr2lineQuery(binary, item_list[3])

        string += line_info + "\n"

        outfile.write(string)
        print string,


    outfile.close()



        





if __name__ == "__main__":
    main()
