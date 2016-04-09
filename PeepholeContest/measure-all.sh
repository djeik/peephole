#!/bin/bash

set -e

NOFILENAME=0

while (( $# )) ; do
    case "$1" in
        "--")
            shift
            break
            ;;
        "--nofilename")
            NOFILENAME=1
            ;;
    esac
    shift
done

if [ -z "$*" ] ; then
    files="$(find PeepholeBenchmarks -mindepth 1 -maxdepth 1 -type d)"
else
    files="$*"
fi

for bench in $files ; do
    (
    cd $bench
    for f in *.dump ; do
        if [ $NOFILENAME -eq 1 ] ; then
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
