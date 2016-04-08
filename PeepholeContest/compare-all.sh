#!/bin/bash

if [[ `uname` == "CYGWIN_NT-6.1-WOW" ]]
then
    COMPILE_SCRIPT=./cyg_compileall
else
    COMPILE_SCRIPT=./compile-all.sh
fi

set -e

$COMPILE_SCRIPT
./dump-all.sh
./measure-all.sh > tmp1

$COMPILE_SCRIPT -O
./dump-all.sh
./measure-all.sh --nofilename > tmp2

paste tmp1 tmp2
