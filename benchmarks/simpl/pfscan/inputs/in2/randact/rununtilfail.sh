#!/bin/bash

rm -f thrille-sched
touch thrille-sched
rm -f my-schedule
rm -f thrille-randomactive

LD_PRELOAD=/home/jalbert/thrille/trunk/thrille/bin/liblockrace.so ../../benchmarks/llvm-compilable/pbzip2-0.9.4/pbzip2 -f -k -p2 -o ../../benchmarks/llvm-compilable/pbzip2-0.9.4/smallinput

while [ $? != "3" ]; do
    LD_PRELOAD=/home/jalbert/thrille/trunk/thrille/bin/librandact.so ../../benchmarks/llvm-compilable/pbzip2-0.9.4/pbzip2 -f -k -p2 -o ../../benchmarks/llvm-compilable/pbzip2-0.9.4/smallinput

done

