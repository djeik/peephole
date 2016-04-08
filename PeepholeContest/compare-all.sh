#!/bin/bash

OPT_ONLY=0
NOFILENAME=

while (( $# )) ; do
    case "$1" in
        "--opt-only")
            OPT_ONLY=1
            ;;
        "--nofilename")
            NOFILENAME="--nofilename"
            ;;
    esac

    shift
done

if [[ `uname` == "CYGWIN_NT-6.1-WOW" ]]
then
    COMPILE_SCRIPT=./cyg_compileall
else
    COMPILE_SCRIPT=./compile-all.sh
fi

set -e

if [ $OPT_ONLY -eq 1 ] ; then
    $COMPILE_SCRIPT -O
    ./dump-all.sh > /dev/null
    ./measure-all.sh $NOFILENAME | tee results
    exit
fi

$COMPILE_SCRIPT
./dump-all.sh
./measure-all.sh > tmp1

$COMPILE_SCRIPT -O
./dump-all.sh
./measure-all.sh --nofilename > tmp2

paste tmp1 tmp2 | tee results
