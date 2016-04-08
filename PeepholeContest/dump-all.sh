#!/bin/bash

set -e

JASMIN="java -jar $PEEPDIR/jasmin.jar"
DJAS=$PEEPDIR/djas

for bench in $(find PeepholeBenchmarks -type d -depth 1) ; do
    (
    cd $bench
    for f in *.j ; do
        $JASMIN $f
        $DJAS -w ${f%.*}.class > ${f%.*}.dump
    done
    )
done
