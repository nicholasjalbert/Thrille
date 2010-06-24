#!/bin/bash

# This script will automatically set up LLVM for you Requires that the LLVM
# binaries be in your path--download the 2.6 binary package from
# http://llvm.org/releases/
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#
#

# Check that llvm binaries are in your path
TMP=$(which llvm-g++)
if [ $? -ne 0 ]
then
    echo
    echo "The 2.6 LLVM binaries (llvm-g++, llvm-gcc, etc) must be on your path."
    echo "Retrieve the binary package from http://llvm.org/releases"
    echo
    exit
fi

# Check that THRILLE_ROOT is defined
if [ -z "$THRILLE_ROOT" ]
then
    echo "Environment variable THRILLE_ROOT must be set"
    exit 1
fi

cp $THRILLE_ROOT/scripts/config/zip/llvm-2.6.tar.gz $THRILLE_ROOT/src/
cd $THRILLE_ROOT/src

echo "Cleaning any old versions of LLVM 2.6..."
rm -rf $THRILLE_ROOT/src/llvm-2.6

echo "Unzipping LLVM 2.6..."
tar xzf llvm-2.6.tar.gz
rm llvm-2.6.tar.gz

echo "Compiling LLVM 2.6 (Be Patient--this takes a while)..."
BLOG=llvm2.6-build.log
cd llvm-2.6
./configure > $BLOG 2>&1
make >> $BLOG 2>&1
echo "Build log can be found at $THRILLE_ROOT/src/llvm-2.6/$BLOG"

echo "Compiling LoadStore transform..."
cp -r $THRILLE_ROOT/src/LoadStore $THRILLE_ROOT/src/llvm-2.6/lib/Transforms/ >> /dev/null 2>&1
cd $THRILLE_ROOT/src/llvm-2.6/lib/Transforms/LoadStore
make >> /dev/null 2>&1

echo "Finishing up..."
cp $THRILLE_ROOT/src/llvm-2.6/Release/lib/LLVMLoadStore.so $THRILLE_ROOT/bin



