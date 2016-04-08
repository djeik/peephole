#!/bin/bash

set -e

JASMIN="java -jar $PEEPDIR/jasmin.jar"
DJAS=$PEEPDIR/djas

for bench in $(find PeepholeBenchmarks -mindepth 1 -maxdepth 1 -type d) ; do
    (
    cd $bench
    for f in *.j ; do
        $JASMIN $f
        $DJAS -w ${f%.*}.class > ${f%.*}.dump
    done
    )
done
