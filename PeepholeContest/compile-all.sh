#!/bin/bash

set -e

JOOS=$PEEPDIR/joos

for bench in $(find PeepholeBenchmarks -type d -depth 1) ; do
    (
    export CLASSPATH=$(pwd)/jooslib.jar
    cd $bench
    $JOOS $@ *.java $(if [ -n "$(find . -name '*.joos')" ] ; then
        echo *.joos
    fi)
    echo 'compiled' $bench
    )
done
