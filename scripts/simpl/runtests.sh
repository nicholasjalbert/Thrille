#!/bin/bash
set -e

if [ -z "$THRILLE_ROOT" ]
then
    echo "Environment variable THRILLE_ROOT must be set"
    exit 1
fi


cd $THRILLE_ROOT/scripts/
python runfunctionaltests.py -a
python simpl/src/util/statgenerator.py -test
python simpl/src/common/schedule.py
python simpl/src/chess/searchstack.py
python simpl/src/tinertia/tinertia.py -test
python simpl/src/chess/chess.py -test
