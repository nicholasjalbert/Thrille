#!/bin/bash
set -e

if [ -z "$THRILLE_ROOT" ]
then
    echo "Environment variable THRILLE_ROOT must be set"
    exit 1
fi


cd $THRILLE_ROOT/src/serializer
make
cd $THRILLE_ROOT/src/randomschedule
make
cd $THRILLE_ROOT/src/strictserial
make
cd $THRILLE_ROOT/src/relaxedserial
make
cd $THRILLE_ROOT/src/addrserial
make
cd $THRILLE_ROOT/src/chessserial
make
cd $THRILLE_ROOT/src/relaxedtester
make
cd $THRILLE_ROOT/src/dfailserial
make
cd $THRILLE_ROOT/src/randact
make
cd $THRILLE_ROOT/src/racer
make
cd $THRILLE_ROOT/src/lockrace
make
