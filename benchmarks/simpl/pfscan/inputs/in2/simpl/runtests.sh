#!/bin/bash
set -e

cd $THRILLE_ROOT/scripts/
python runfunctionaltests.py -a
python simpl/fwdrev/fwdrev.py
python simpl/fwdrev/simpl.py -test
