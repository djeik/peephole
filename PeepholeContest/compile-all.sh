#!/bin/bash

set -e

JOOS=$PEEPDIR/joos

(cd JOOSA-src ; make)

for bench in $(find PeepholeBenchmarks -mindepth 1 -maxdepth 1 -type d) ; do
    (
    export CLASSPATH=$(pwd)/jooslib.jar
    cd $bench
    $JOOS $@ *.java $(if [ -n "$(find . -name '*.joos')" ] ; then
        echo *.joos
    fi)
    echo 'compiled' $bench
    )
done
