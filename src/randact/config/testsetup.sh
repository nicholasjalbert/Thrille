#!/bin/bash -x

mv $THRILLE_ROOT/src/serializer/config/thrille-print $THRILLE_ROOT/src/serializer/config/thrille-print.old
echo "0" > $THRILLE_ROOT/src/serializer/config/thrille-print
echo "LOCK" > $THRILLE_ROOT/src/randact/thrille-randomactive
echo "0x4" >> $THRILLE_ROOT/src/randact/thrille-randomactive
echo "0x5" >> $THRILLE_ROOT/src/randact/thrille-randomactive

