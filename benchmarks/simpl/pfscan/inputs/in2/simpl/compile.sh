#!/bin/bash
set -e

cd $THRILLE_ROOT/src/serializer
make
cd $THRILLE_ROOT/src/randomschedule
make
cd $THRILLE_ROOT/src/strictserial
make
cd $THRILLE_ROOT/src/relaxedserial
make
cd $THRILLE_ROOT/src/randact
make
cd $THRILLE_ROOT/src/racedetect
make
cd $THRILLE_ROOT/src/lockrace
make
