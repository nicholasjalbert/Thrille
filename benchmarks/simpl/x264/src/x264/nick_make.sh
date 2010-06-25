#!/bin/bash

make -f nick.mak
make
mkdir -p ../../bin/
cp x264 ../../bin/x264-e1
