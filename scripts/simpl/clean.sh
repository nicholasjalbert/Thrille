#!/bin/bash
set -e

if [ -z "$THRILLE_ROOT" ]
then
    echo "Environment variable THRILLE_ROOT must be set"
    exit 1
fi

rm -rf $THRILLE_ROOT/obj/*
rm -f $THRILLE_ROOT/bin/lib*
