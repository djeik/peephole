#!/bin/bash

if [[ `uname` == "CYGWIN_NT-6.1-WOW" ]]
then
    COMPILE_SCRIPT=./cyg_compileall
else
    COMPILE_SCRIPT=./compile-all.sh
fi

set -e

if [ "$1" = "--opt-only" ] ; then
    $COMPILE_SCRIPT -O
    ./dump-all.sh
    ./measure-all.sh | tee results
    exit
fi

$COMPILE_SCRIPT
./dump-all.sh
./measure-all.sh > tmp1

$COMPILE_SCRIPT -O
./dump-all.sh
./measure-all.sh --nofilename > tmp2

paste tmp1 tmp2 | tee results
