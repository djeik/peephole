#!/bin/bash

set -e

for bench in $(find PeepholeBenchmarks -type d -depth 1) ; do
    (
    cd $bench
    for f in *.dump ; do
        if [ "$1" = "--nofilename" ] ; then
            name=""
        else
            name="$f"
        fi

        echo $name $(
            grep -o 'code_length [[:digit:]]*' $f | cut -d " " -f2 |
            paste -s -d + - | bc
        )
    done
    )
done
